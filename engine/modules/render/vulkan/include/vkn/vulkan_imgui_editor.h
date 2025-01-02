#pragma once
#include "editor/editor_system.h"
#include "render/graph/frame_graph.h"
#include "vkn/vulkan_api.h"
namespace vkn {
	using api::FrameGraph;
	using api::RenderPassContext;
	using api::RenderPassBuilder;
	class VulkanImguiEditor : public api::EditorSystem {
	public:
		void Initialize() override;
		void Finalize() override;
		void Render(FrameGraph& graph, RenderPassContext& ctx) override;
		ImTextureID AddTexture(ImageViewPtr imageview, SamplerPtr sampler, ResourceState state) override;

		static VulkanImguiEditor* Ptr() {
			return (VulkanImguiEditor*)api::EditorSystem::Ptr();
		};
	};
}