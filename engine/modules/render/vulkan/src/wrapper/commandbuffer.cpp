#include "vkn/wrapper/commandbuffer.h"
namespace vkn {
	void CommandBuffer::Reset()
	{
		vkResetCommandBuffer(mPtr, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}
	void CommandBuffer::BeginRecord(VkCommandBufferUsageFlags flag)
	{
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,//sType
			nullptr,									//pNext
			flag									    //flags
		};
		vkBeginCommandBuffer(mPtr, &beginInfo);
	}
	void CommandBuffer::EndRecord()
	{
		vkEndCommandBuffer(mPtr);
	}
	void CommandBuffer::CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkBufferCopy copy = {};
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = size;
		vkCmdCopyBuffer(mPtr, srcBuffer, dstBuffer, 1, &copy);
	}
	void CommandBuffer::Submit(VkQueue& queue, VkFence fence)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mPtr;
		vkQueueSubmit(queue, 1, &submitInfo, fence);
	}
	void CommandBuffer::BindVertexBuffer(VkBuffer buffer, uint32_t offset)
	{
		VkDeviceSize offsets[] = { offset };
		vkCmdBindVertexBuffers(mPtr, 0, 1, &buffer, offsets);
	}
	void CommandBuffer::BindIndexBuffers(VkBuffer buffer, uint32_t offset, VkIndexType type)
	{
		vkCmdBindIndexBuffer(mPtr, buffer, offset, type);
	}
}

