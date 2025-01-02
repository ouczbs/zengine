#include "vkn/wrapper/commandpool.h"
#include "vkn/wrapper/device.h"
#include "vkn/wrapper/queue.h"
#include "zlog.h"
namespace vkn {
	CommandPool::CommandPool(Device& device, VkCommandPoolCreateFlags queueFlags, uint32_t queueIndex)
		:mPtr(nullptr)
		,mDevice(device)
	{
		VkCommandPoolCreateInfo pCreateInfo{
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
			queueFlags,
			queueIndex
		};
		vkCreateCommandPool(device.Ptr(), &pCreateInfo, nullptr, &mPtr);
	}
	CommandPool::~CommandPool()
	{
		for (auto cb : mPool) {
			FreeBuffer(cb.Ptr());
		}
		mPool.clear();
	}
	VkCommandBuffer CommandPool::AllocateBuffer(VkCommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo allocInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, //sType
			nullptr,									    //pNext
			mPtr,                                           //commandPool
			level,                                          //level 
			1,                                              //commandBufferCount 
		};
		VkCommandBuffer cmd;
		vkAllocateCommandBuffers(mDevice.Ptr(), &allocInfo, &cmd);
		return cmd;
	}
	void CommandPool::FreeBuffer(VkCommandBuffer& buf)
	{
		vkFreeCommandBuffers(mDevice.Ptr(), mPtr, 1, &buf);
		buf = nullptr;
	}
	CommandBuffer CommandPool::Pop()
	{
		if (mPool.empty()) {
			return AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}
		CommandBuffer buffer = mPool.back();
		mPool.pop_back();
		return buffer;
	}
	void CommandPool::PopList(vector<VkCommandBuffer>& list, int size)
	{
		list.reserve(size);
		for (int i = 0; i < size; i++) {
			list.push_back(AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
		}
	}
	void CommandPool::Push(CommandBuffer& cmd)
	{
		cmd.Reset();
		mPool.push_back(cmd);
	}
}

