#pragma once
#include "type.h"
#include "render/window.h"
#include <SDL2/SDL_vulkan.h>
namespace vkn {
	class Device;
	struct VulkanContext;
	using api::TextureDesc;
	struct VULKAN_API VulkanWindowArgs {
		uint32_t frames;
		uint32_t width;
		uint32_t height;
		uint32_t framesInFlight;
		VkFormat imageFormat;
		VkColorSpaceKHR imageColorSpace;
		VkPresentModeKHR presentMode;
		VkImageUsageFlags imageUsage;
		VkExtent2D EnableImageExtent2D(VkSurfaceCapabilitiesKHR& capabilities);
		static VulkanWindowArgs Default(uint32_t frames);
	};
	class VulkanSwapchain {
	private:
		friend class VulkanWindow;
		Device& mDevice;
		VkSwapchainKHR mPtr;
		uint32_t mFrames;
		pmr::vector<TextureDesc>	 mSurfaces{ GlobalPool() };
		pmr::vector<VkCommandBuffer> mCommands{ GlobalPool() };
		pmr::vector<VkFence>		 mFences{ GlobalPool() };
		pmr::vector<VkSemaphore>	 mSemaphores{ GlobalPool() };
	public:
		VulkanSwapchain(Device& device, VkSurfaceKHR surface, VulkanWindowArgs& args);
		void Aquire(VulkanContext& ctx);
		void Present(VulkanContext& ctx);
		VkCommandBuffer GetTransferCommand(uint32_t frame) { return mCommands[frame + mFrames]; };
	};
	class VULKAN_API VulkanWindow : public api::Window {
	private:
		VulkanSwapchain* mSwapchain;
	public:
		void* operator new(size_t size) {
			return ::operator new(size, GlobalPool());
		}
		void operator delete(void* p) {}
	public:
		using api::Window::Window;
		bool CreateRender(VulkanWindowArgs& args);
		static VulkanWindow* Ptr() {
			return (VulkanWindow*)api::Window::Ptr();
		}
		void Aquire(VulkanContext& ctx) { mSwapchain->Aquire(ctx); };
		void Present(VulkanContext& ctx) { mSwapchain->Present(ctx); };
		VkCommandBuffer GetTransferCommand(uint32_t frame) { return mSwapchain->GetTransferCommand(frame); };
		VulkanSwapchain* Swapchain() {
			return mSwapchain;
		}
	};
}