#pragma once
#include "vulkan_context.h"
#include "asset/res/guid.h"
#include "backend.h"
#ifdef API_DEBUG
	#define gLogSemaphore(...) //zlog::info(__VA_ARGS__)
#else
	#define gLogSemaphore(...) 
#endif
namespace vkn {
	class VulkanWindow;
	using api::Guid;
	using api::Mesh;
	using api::Shader;
	using api::Material;
	using api::MaterialInstance;
	using api::RenderPassNode;
	using api::RenderPassType;
	using api::RenderPassParams;
	class VULKAN_API VulkanAPI final : public api::RenderAPI{
	private:
		VulkanWindow& window;
		Backend backend;
		table<Guid, MeshVAO>			     MeshTable;
		table<Guid, VulkanPipeline>			 PipelineTable;
		table<size_t, RenderPassInfo>		 RenderPassCache;
		table<Name, RenderPassInfo>			 RenderPassNameCache;
		table<FramebufferKey, VkFramebuffer> FramebufferCache;
	public:
		VulkanAPI(VulkanWindow* pWindow);
		void Init() override;
		void Shutdown() override;

		void SetStaticMesh(Mesh& mesh)override;
		void DrawStaticMesh(Mesh& mesh)override;

		void SetUpMaterialInstance(MaterialInstance& material) override;
		void LoadShader(Shader& shader, size_t passKey)override;

		void CreateBuffer(BufferDesc& desc) override;
		void CreateTexture(TextureDesc& desc)override;
		ImageViewPtr CreateTextureView(TextureViewKey desc)override;
		SamplerPtr   CreateTextureSampler(TextureSampler sampler) override;

		void BeginFrame()override;
		void EndFrame()override;
		void BeginRenderPass(RenderPassNode* node, FnEnterRenderPass callback) override;
		void EndRenderPass(RenderPassNode* node) override;

		VkPipeline	 GetPipeline() { return nullptr; };
		RenderPassInfo* GetRenderPassInfo(Name name, size_t hash);
		RenderPassInfo* GetRenderPassInfo(size_t& hash, const RenderPassKey& config);
		void SetRenderPassInfo(Name name, VkRenderPass pass);
		Backend& GetBackend() {
			return backend;
		}
		static VulkanAPI* Ptr() {
			return (VulkanAPI*)api::RenderAPI::Ptr();
		}
	};
}