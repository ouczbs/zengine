#pragma once
#include "type.h"
#include "render/render_api.h"
namespace vkn {
	using api::BufferDesc;
	using api::RenderPassState;
	struct VulkanContext : public api::RenderContext {
		VkFence		surfaceFence;
		VkFence		transferFence;
		uint8_t     waitFenceNums[8]{ 2,2,2,2,2,2,2,2 };//最多不能超过8个
		VkSemaphore surfaceSemaphore;
		VkSemaphore presentSemaphore;
		VkSemaphore graphSemaphore;
		VkSemaphore transferSemaphore;
		VkCommandBuffer transferCommand;
		VkCommandBuffer surfaceCommand;
		VkCommandBuffer command;
		void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) override;
		void BindIndexBuffer(BufferDesc desc, uint32_t index_stride) override;
		void BindVertexBuffer(BufferDesc desc) override;
		void BindVertexBuffers(uint32_t buffer_count, const BufferDesc* descs, const uint32_t* offsets)override;
		void DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex) override;
		void ExecuteSurfaceBarriers(const ResourceBarrierDesc& desc) override;
		void ExecuteResourceBarriers(const ResourceBarrierDesc& desc) override;
		void UpdateTexture(TextureDesc& texture, const TextureUpdateArgs& update, ResourceState state) override;

		void ClearSurface(VkClearColorValue clearValue);
		void BeginRecord(VkCommandBuffer cmd, VkCommandBufferUsageFlags flag);
		void EndRecord(VkQueue queue);
		void FlushCommand();
		VkCommandBuffer GetTransferCommand();
	};
}