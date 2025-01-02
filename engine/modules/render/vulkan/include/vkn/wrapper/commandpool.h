#pragma once
#include "commandbuffer.h"
namespace vkn {
	class Device;
	class Queue;
	class CommandPool {
	protected:
		VkCommandPool mPtr;
		Device& mDevice;
		pmr::vector<CommandBuffer> mPool;
	public:
		CommandPool(Device& device, VkCommandPoolCreateFlags queueFlags, uint32_t queueIndex);
		~CommandPool();
		VkCommandBuffer AllocateBuffer(VkCommandBufferLevel level);
		void FreeBuffer(VkCommandBuffer& buf);
		VkCommandPool& Ptr() {
			return mPtr;
		};
		CommandBuffer Pop();
		void PopList(vector<VkCommandBuffer>& list, int size);
		void Push(CommandBuffer& cmd);
	};
}