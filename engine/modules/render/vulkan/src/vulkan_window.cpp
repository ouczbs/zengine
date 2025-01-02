#include "vkn/vulkan_window.h"
#include "vkn/wrapper/device.h"
#include "vkn/wrapper/instance.h"
#include "vkn/vulkan_api.h"
#include "vkn/backend.h"
#include "vkn/thread/command_worker.h"
#include "render/asset/texture.h"
#include "asset/resource_system.h"
#include "zlog.h"
#include <algorithm>
#include <tinyimageformat/tinyimageformat_apis.h>
namespace vkn {
	bool VulkanWindow::CreateRender(VulkanWindowArgs& args)
	{
		VulkanAPI* api = VulkanAPI::Ptr();
		Backend& backend = api->GetBackend();
		VkInstance instance = backend.GetInstance().Ptr();
		VkSurfaceKHR surface;
		if (!SDL_Vulkan_CreateSurface(mPtr, instance, &surface)) {
			return false;
		}
		args.width = mWidth;
		args.height = mHeight;
		mSwapchain = new (GlobalPool()) VulkanSwapchain(backend.GetDevice(), surface, args);
		api->context.frameCount = args.framesInFlight;
		api->context.frameNumber = args.framesInFlight;
		api->context.surface = mSwapchain->mSurfaces[0];
		api->graph.InitSurface(mSwapchain->mSurfaces.data(), mSwapchain->mSurfaces.size());
		return true;
	}
	VulkanSwapchain::VulkanSwapchain(Device& device, VkSurfaceKHR surface, VulkanWindowArgs& args)
		: mDevice(device)
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysical(), surface, &capabilities);
		VkExtent2D image_extent = args.EnableImageExtent2D(capabilities);
		VkSwapchainCreateInfoKHR swapchain_create_info = {
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,// VkStructureType                  sType
			nullptr,                                    // const void                     * pNext
			0,                                          // VkSwapchainCreateFlagsKHR        flags
			surface,									// VkSurfaceKHR                     surface
			(uint32_t)args.frames,						// uint32_t                         minImageCount
			args.imageFormat,							// VkFormat                         imageFormat
			args.imageColorSpace,						// VkColorSpaceKHR                  imageColorSpace
			image_extent,                               // VkExtent2D                       imageExtent
			1,                                          // uint32_t                         imageArrayLayers
			args.imageUsage,							// VkImageUsageFlags                imageUsage
			VK_SHARING_MODE_EXCLUSIVE,                  // VkSharingMode                    imageSharingMode
			0,                                          // uint32_t                         queueFamilyIndexCount
			nullptr,                                    // const uint32_t                 * pQueueFamilyIndices
			capabilities.currentTransform,              // VkSurfaceTransformFlagBitsKHR    preTransform
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,          // VkCompositeAlphaFlagBitsKHR      compositeAlpha
			args.presentMode,							// VkPresentModeKHR                 presentMode
			VK_TRUE,                                    // VkBool32                         clipped
			VK_NULL_HANDLE                              // VkSwapchainKHR                   oldSwapchain
		};
		V(vkCreateSwapchainKHR(device.Ptr(), &swapchain_create_info, nullptr, &mPtr));
		mFrames = args.framesInFlight;
		uint32_t flightFrames = args.framesInFlight * 2;
		pmr::vector<VkImage> swapchain_images{ FramePool() };
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device.Ptr(), mPtr, &imageCount, nullptr);
		swapchain_images.resize(imageCount);
		vkGetSwapchainImagesKHR(device.Ptr(), mPtr, &imageCount, swapchain_images.data());
		mFences.reserve(flightFrames);
		mSurfaces.reserve(imageCount);
		mCommands.reserve(flightFrames);
		mSemaphores.reserve(flightFrames + mFrames);
		TextureDesc desc{};
		desc.width = args.width;
		desc.height = args.height;
		desc.format = TinyImageFormat_FromVkFormat((TinyImageFormat_VkFormat)args.imageFormat);
		desc.state = ResourceState::UNDEFINED;
		desc.sampleCount = SampleCount::SAMPLE_COUNT_1;
		desc.arraySize = 1;
		desc.mipLevel = 1;
		desc.depth = 1;
		desc.dimension = TextureDimension::TEX_2D;
		for (int i = 0; i < mFrames; i++) {
			desc.image = swapchain_images[i];
			mSurfaces.push_back(desc);
			mSemaphores.push_back(mDevice.CreateSemaphore());
			mSemaphores.push_back(mDevice.CreateSemaphore());
			mSemaphores.push_back(mDevice.CreateSemaphore());
			mCommands.push_back(Backend::RenderWorker->AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
			mCommands.push_back(Backend::RenderWorker->AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
			mFences.push_back(mDevice.CreateFence(VK_FENCE_CREATE_SIGNALED_BIT));
			mFences.push_back(mDevice.CreateFence(VK_FENCE_CREATE_SIGNALED_BIT));
		}
		for (int i = mFrames; i < imageCount; i++) {
			desc.image = swapchain_images[i];
			mSurfaces.push_back(desc);
		}
	}
	void VulkanSwapchain::Aquire(VulkanContext& ctx)
	{
		VkFence surfaceFence = mFences[ctx.frame];
		VkFence transferFence = mFences[ctx.frame + mFrames];
		VkSemaphore surfaceSemaphore = mSemaphores[ctx.frame];
		ctx.surfaceCommand = mCommands[ctx.frame];
		ctx.surfaceFence = surfaceFence;
		ctx.transferFence = transferFence;
		ctx.surfaceSemaphore = surfaceSemaphore;
		ctx.presentSemaphore = mSemaphores[ctx.frame + mFrames];
		ctx.transferSemaphore = mSemaphores[ctx.frame + mFrames + mFrames];
		VkFence waitFence[2] = { surfaceFence, transferFence };
		uint32_t fenceNum = ctx.waitFenceNums[ctx.frame];
		V(vkWaitForFences(mDevice.Ptr(), fenceNum, waitFence, VK_TRUE, UINT64_MAX));
		vkAcquireNextImageKHR(mDevice.Ptr(), mPtr, UINT64_MAX, surfaceSemaphore, VK_NULL_HANDLE, &ctx.presentFrame);
		vkResetFences(mDevice.Ptr(), fenceNum, waitFence);
		ctx.surface = mSurfaces[ctx.presentFrame];
		ctx.graphSemaphore = nullptr;
		ctx.waitFenceNums[ctx.frame] = 0;
		gLogSemaphore("aquire------------:: {:#x} transfer::{:#x}", (uintptr_t)surfaceSemaphore, (uintptr_t)ctx.transferSemaphore);
	}
	void VulkanSwapchain::Present(VulkanContext& ctx)
	{
		gLogSemaphore("present+++++++++++:: {:#x}", (uintptr_t)ctx.presentSemaphore);
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pWaitSemaphores = &ctx.graphSemaphore;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pSwapchains = &mPtr;
		presentInfo.swapchainCount = 1;
		presentInfo.pImageIndices = &ctx.presentFrame;
		presentInfo.pResults = VK_NULL_HANDLE;
		zlog::flush();
		Backend::RenderWorker->Present(presentInfo);
		ctx.frame = (ctx.frame + 1) % mFrames;
	}
	VkExtent2D VulkanWindowArgs::EnableImageExtent2D(VkSurfaceCapabilitiesKHR& capabilities)
	{
		VkExtent2D image_extent{ width , height };
		if (capabilities.currentExtent.width != 0xFFFFFFFF)
		{
			image_extent = capabilities.currentExtent;
		}
		else
		{
			image_extent.width = std::clamp(image_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			image_extent.height = std::clamp(image_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}
		return image_extent;
	}
	VulkanWindowArgs VulkanWindowArgs::Default(uint32_t frames)
	{
		return {
			.frames = frames,
			.framesInFlight = 2,
			.imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
			.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			.presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
			.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		};
	}
}