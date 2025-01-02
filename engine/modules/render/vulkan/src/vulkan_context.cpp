#include "vkn/backend.h"
#include "vkn/vulkan_api.h"
#include "vkn/thread/command_worker.h"
#include "vkn/vulkan_api_help.h"
#include "vkn/thread/buffer_worker.h"
#include "vkn/vulkan_window.h"
#include "zlog.h"
namespace vkn {
	void VulkanContext::SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		VkRect2D scissor = {
			.offset{ .x = (int32_t)x, .y = (int32_t)y},
			.extent {.width = width,.height = height}
		};
		vkCmdSetScissor(command, 0, 1, &scissor);
	}
	void VulkanContext::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
	{
		VkViewport viewport = {
			.x = x,
			.y = y,
			.width = width,
			.height = height,
			.minDepth = min_depth,
			.maxDepth = max_depth
		};
		vkCmdSetViewport(command, 0, 1, &viewport);
	}
	void VulkanContext::BindIndexBuffer(BufferDesc desc, uint32_t index_stride)
	{
		VkIndexType vk_index_type =
			(sizeof(uint16_t) == index_stride) ?
			VK_INDEX_TYPE_UINT16 : ((sizeof(uint8_t) == index_stride) ? VK_INDEX_TYPE_UINT8_EXT : VK_INDEX_TYPE_UINT32);
		vkCmdBindIndexBuffer(command, (VkBuffer)desc.buffer, 0, vk_index_type);
	}
	void VulkanContext::BindVertexBuffer(BufferDesc desc)
	{
		VkBuffer vertexBuffers[] = { (VkBuffer)desc.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);
	}
	void VulkanContext::BindVertexBuffers(uint32_t buffer_count, const BufferDesc* descs, const uint32_t* offsets)
	{
		VkBuffer vkBuffers[MAX_BUFFER_BIND_NUM] = { 0 };
		VkDeviceSize vkOffsets[MAX_BUFFER_BIND_NUM] = { 0 };
		for (uint32_t i = 0; i < buffer_count; i++) {
			vkBuffers[i] = (VkBuffer)descs[i].buffer;
			vkOffsets[i] = offsets[i];
		}
		vkCmdBindVertexBuffers(command, 0, buffer_count, vkBuffers, vkOffsets);
	}
	void VulkanContext::DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
	{
		vkCmdDrawIndexed(command, index_count, 1, first_index, first_vertex, 0);
	}
	void VulkanContext::ClearSurface(VkClearColorValue clearValue)
	{
		// 条件满足时，手动清除附件
		VkClearAttachment clearAttachment = {};
		clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // 仅清除颜色附件
		clearAttachment.colorAttachment = 0;  // 附件索引
		clearAttachment.clearValue.color = clearValue;  // 传递清除值

		VkClearRect clearRect = {};
		clearRect.rect.offset = { 0, 0 };  // 清除区域
		clearRect.rect.extent = { surface.width, surface.height };  // 渲染区域的大小
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;  // 默认清除第一个层级

		// 使用 vkCmdClearAttachments 清除颜色附件
		vkCmdClearAttachments(command, 1, &clearAttachment, 1, &clearRect);
	}
	void VulkanContext::BeginRecord(VkCommandBuffer cmd, VkCommandBufferUsageFlags flag)
	{
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,//sType
			nullptr,									//pNext
			flag									    //flags
		};
		vkBeginCommandBuffer(cmd, &beginInfo);
	}

	void VulkanContext::EndRecord(VkQueue queue)
	{
		vkEndCommandBuffer(surfaceCommand);
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };  // 等待渲染阶段
		VkSemaphore waitSemaphores[] = { graphSemaphore ? graphSemaphore : surfaceSemaphore };
		VkSemaphore signalSemaphores[] = { presentSemaphore };// 渲染完成信号量
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &surfaceCommand;

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		gLogSemaphore("-----wait {:#x}", (uintptr_t)waitSemaphores[0]);
		gLogSemaphore("+++++sign {:#x}", (uintptr_t)signalSemaphores[0]);
		vkQueueSubmit(queue, 1, &submitInfo, surfaceFence);
		graphSemaphore = presentSemaphore;
	}
	void VulkanContext::FlushCommand()
	{
		if (waitFenceNums[frame])
			return;
		waitFenceNums[frame] = 1;
		if (transferCommand) {
			vkEndCommandBuffer(transferCommand);
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &transferCommand;
			submitInfo.pSignalSemaphores = &transferSemaphore;
			submitInfo.signalSemaphoreCount = 1;
			gLogSemaphore("+++++sign {:#x}", (uintptr_t)transferSemaphore);
			vkQueueSubmit(Backend::RenderWorker->GetQueue().Ptr(), 1, &submitInfo, transferFence);
			transferCommand = nullptr;
			waitFenceNums[frame] = 2;
		}
	}
	VkCommandBuffer VulkanContext::GetTransferCommand()
	{
		switch (renderPassState)
		{
		case api::RenderPassState::BeginRecord:
			return command;
		case api::RenderPassState::BeginRender:
		{
			if (!transferCommand) {
				transferCommand = VulkanWindow::Ptr()->GetTransferCommand(frame);
				BeginRecord(transferCommand, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			}
			return transferCommand;
		}
		case api::RenderPassState::Present:
			return surfaceCommand;
		default:
			return nullptr;
		}
	}
	void VulkanContext::ExecuteSurfaceBarriers(const ResourceBarrierDesc& desc)
	{
		BeginRecord(surfaceCommand, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		ExecuteResourceBarriers(desc);
		EndRecord(Backend::RenderWorker->GetQueue().Ptr());
	}
	void VulkanContext::ExecuteResourceBarriers(const ResourceBarrierDesc& desc) {
		pmr::vector<VkBufferMemoryBarrier> bufferBarriers{ FramePool() };
		bufferBarriers.reserve(desc.bufferBarriersCount);
		pmr::vector<VkImageMemoryBarrier> imageBarriers{ FramePool() };
		imageBarriers.reserve(desc.textureBarriersCount);
		VkPipelineStageFlags srcStageMask = 0, dstStageMask = 0;
		for (uint32_t i = 0; i < desc.textureBarriersCount; i++)
		{
			auto& barrier = desc.pTextureBarriers[i];
			auto desc = vkApiGetTextureTransition(srcStageMask, dstStageMask, barrier);
			//zlog::info("textureBarrier::{:#x} {} {}::{}", (uintptr_t)barrier.mTexture.image,(uint8_t)renderPassState,(uint8_t)barrier.mSrcState, (uint8_t)barrier.mDstState);
			imageBarriers.push_back(desc);
		}
		if (dstStageMask == 0) {
			dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		vkCmdPipelineBarrier(GetTransferCommand(), srcStageMask, dstStageMask, 0, 0, NULL, 0, NULL, imageBarriers.size(), imageBarriers.data());
	}
	void VulkanContext::UpdateTexture(TextureDesc& texture, const TextureUpdateArgs& update, ResourceState state)
	{
		if (!texture.id) {
			VulkanAPI::Ptr()->graph.AcquireTexture(texture);
		}
		if (texture.state != state) {
			TextureBarrier barrier{};
			barrier.mSrcState = texture.state;
			barrier.mDstState = state;
			barrier.mTexture = texture;
			ResourceBarrierDesc desc{};
			desc.pTextureBarriers = &barrier;
			desc.textureBarriersCount = 1;
			ExecuteResourceBarriers(desc);
		}
		uint32_t texelBytes = texture.format == TinyImageFormat_R8_UNORM ? 1 : 4;
		uint32_t size = update.width * update.height * texelBytes;
		ImageUpdator updator{};
		updator.args = update;
		updator.size = size;
		updator.image = (VkImage)texture.image;
		updator.command = GetTransferCommand();
		Backend::TransferWorker->UpdateImage(updator);
	}
}
