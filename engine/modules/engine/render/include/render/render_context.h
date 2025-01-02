#pragma once
#include "type.h"
#include "render_type.h"
namespace api {
	struct RenderContext;
	struct FRenderView {
		RenderContext* context;
	};
	enum class RenderPassState : uint8_t {
		Default,
		BeginRecord,
		BeginRender,
		Present,
	};
	struct RenderContext {
		pmr::vector<FRenderView> views;
		TextureDesc surface;
		uint32_t    frameCount{0};
		uint32_t	frame{ 0 };
		uint32_t	presentFrame{ 0 };
		uint32_t    frameNumber{ 0 };
		RenderPassState renderPassState{ RenderPassState::Default };
		virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
		virtual void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void BindIndexBuffer(BufferDesc desc, uint32_t index_stride) = 0;
		virtual void BindVertexBuffer(BufferDesc buffers) = 0;
		virtual void BindVertexBuffers(uint32_t buffer_count, const BufferDesc* buffers, const uint32_t* offsets) = 0;
		virtual void DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex) = 0;
		virtual void ExecuteSurfaceBarriers(const ResourceBarrierDesc& desc) = 0;
		virtual void ExecuteResourceBarriers(const ResourceBarrierDesc& desc) = 0;
		virtual void UpdateTexture(TextureDesc& texture, const TextureUpdateArgs& update, ResourceState state) = 0;
	};
}