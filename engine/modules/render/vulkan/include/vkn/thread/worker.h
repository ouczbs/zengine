#pragma once
#include "std/thread.h"
#include "vkn/wrapper/commandpool.h"
#include "vkn/wrapper/queue.h"
#include "vkn/wrapper/device.h"
namespace vkn {
	using zstd::channel;
	template<typename value_type, typename Worker>
	class ThreadWorker : public zstd::ThreadWorker<value_type, Worker> {
	protected:
		Name mName;
		Device& mDevice;
		Queue& mQueue;
		CommandPool mCommandPool;
	public:
		ThreadWorker(Name name, Device& device, Queue& queue, VkCommandPoolCreateFlags queueFlags)
			: zstd::ThreadWorker<value_type, Worker>(64)
			, mName(name)
			, mDevice(device)
			, mQueue(queue)
			, mCommandPool(device, queueFlags, queue.QueueFamilyIndex())
		{}
		CommandPool& GetCommandPool() {
			return mCommandPool;
		}
		Queue& GetQueue() {
			return mQueue;
		}
		VkCommandBuffer AllocateBuffer(VkCommandBufferLevel level) {
			return mCommandPool.AllocateBuffer(level);
		}
	};
}