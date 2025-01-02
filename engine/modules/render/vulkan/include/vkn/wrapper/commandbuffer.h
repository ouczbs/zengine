#pragma once
#include "vkn/type.h"
namespace vkn {
	class CommandBuffer {
	protected:
		VkCommandBuffer mPtr;
	public:
		CommandBuffer() : mPtr(nullptr) {};
		CommandBuffer(VkCommandBuffer ptr) : mPtr(ptr) {};
		VkCommandBuffer& Ptr() {
			return mPtr;
		};
		void Reset();
		void BeginRecord(VkCommandBufferUsageFlags flag);
		void EndRecord();
		void CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void Submit(VkQueue& queue,VkFence fence);
		void BindVertexBuffer(VkBuffer buffer, uint32_t offset);
		void BindIndexBuffers(VkBuffer buffer, uint32_t offset, VkIndexType type);
	};
}