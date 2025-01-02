#include "vkn/vulkan_imgui_editor.h"
#include "vkn/vulkan_window.h"
#include "vkn/vulkan_api_help.h"
#include "vkn/backend.h"
#include "vkn/wrapper/device.h"
#include "vkn/wrapper/instance.h"
#include "vkn/wrapper/queue.h"
#include "vkn/vulkan_ui_system.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_sdl2.h"
#include "data/global.h"
#include "event/event_system.h"
#include "tinyimageformat/tinyimageformat_apis.h"
namespace vkn {
	using namespace api;
	static Name ImguiPassName{"ImguiPass"};
	static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;
	// Vulkan的ImGui接入比较麻烦，参考教程: https://frguthmann.github.io/posts/vulkan_imgui/
	VkDescriptorPool CreateDescriptorPool(VkDevice device) {
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VkDescriptorPool descriptorPool;
		vkCreateDescriptorPool(device, &pool_info, VK_NULL_HANDLE, &descriptorPool);
		return descriptorPool;
	}
	void VulkanImguiEditor::Initialize()
	{
		VulkanAPI* API = VulkanAPI::Ptr();
		VulkanWindow* window = VulkanWindow::Ptr();
		Backend& backend = API->GetBackend();
		Queue* pQueue = backend.GetDevice().GetQueue(Queue::RenderQueue);
		VkDescriptorPool descriptorPool = CreateDescriptorPool(backend.GetDevice().Ptr());
		TextureDesc surface = API->context.surface;
		auto renderInfo = API->GetRenderPassInfo(VulkanUISystem::UIPassName, 0);
		VkRenderPass renderPass = renderInfo->pass;

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = backend.GetInstance().Ptr();
		init_info.PhysicalDevice = backend.GetDevice().GetPhysical();
		init_info.Device = backend.GetDevice().Ptr();
		init_info.QueueFamily = pQueue->QueueFamilyIndex();
		init_info.Queue = pQueue->Ptr();
		init_info.DescriptorPool = descriptorPool;
		init_info.MinImageCount = 2;
		init_info.ImageCount = API->context.frameCount;
		init_info.RenderPass = renderPass;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.Subpass = 0;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = VK_NULL_HANDLE;
		ImGui_ImplVulkan_Init(&init_info);
	}
	void VulkanImguiEditor::Finalize()
	{

	}
	ImTextureID VulkanImguiEditor::AddTexture(ImageViewPtr imageview, SamplerPtr sampler, ResourceState state)
	{
		VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture((VkSampler)sampler, (VkImageView)imageview, vkApiGetImageLayout(state));
		return reinterpret_cast<ImTextureID>(descriptorSet);
	}
	void VulkanImguiEditor::Render(FrameGraph& graph, RenderPassContext& context)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		RenderEditorContext editorContext{ .editor = this, .frame = context->frame, .frameCount = context->frameCount };
		for (auto win : mWindows)
		{
			win->Draw(graph, editorContext);
		}

		ImGui::Render();
		VulkanContext& ctx = *(VulkanContext*)context.parent;
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ctx.command);
		ImGuiIO& io = ImGui::GetIO();
		// 更新并渲染平台窗口
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, ctx.command);
		}
	}
}
