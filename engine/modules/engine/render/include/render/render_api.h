#pragma once
#include "pmr/frame_allocator.h"
#include "render_context.h"
#include "graph/frame_graph.h"
namespace api {
	class Mesh;
	class Shader;
	class Material;
	class MaterialInstance;
	class RenderPassNode;
	class RENDER_API RenderAPI
	{	
	public:
		RenderContext&  context;
		FrameGraph		graph;
		RenderAPI(RenderContext* ctx);
		virtual ~RenderAPI();
		SINGLETON_IMPL(RenderAPI)
	public:
		void* operator new(size_t size) {
			return ::operator new(size, GlobalPool());
		}
		void operator delete(void* p) {}
	public:

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void SetStaticMesh(Mesh& mesh) = 0;
		virtual void DrawStaticMesh(Mesh& mesh) = 0;

		virtual void SetUpMaterialInstance(MaterialInstance& material) = 0;
		virtual void LoadShader(Shader& shader, size_t passKey) = 0;

		virtual void CreateBuffer(BufferDesc& desc) = 0;
		virtual void CreateTexture(TextureDesc& desc) = 0;
		virtual ImageViewPtr CreateTextureView(TextureViewKey desc) = 0;
		virtual SamplerPtr   CreateTextureSampler(TextureSampler sampler) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void RenderView(FRenderView& view);
		using FnEnterRenderPass = void(*)(RenderPassNode*);
		virtual void BeginRenderPass(RenderPassNode* node, FnEnterRenderPass callback) = 0;
		virtual void EndRenderPass(RenderPassNode* node) = 0;
		void	Render();
	};
}