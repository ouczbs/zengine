#define VMA_IMPLEMENTATION
#include "vkn/backend.h"
#include "vkn/thread/command_worker.h"
#include "vkn/thread/buffer_worker.h"
#include "vkn/wrapper/queue.h"
#include "vkn/wrapper/device.h"
#include "vkn/vulkan_api_help.h"
#include "vkn/vulkan_context.h"
#include "meta/variant.h"
#include "zlog.h"
#define MAX_TRANSFER_SYNC_FENCES  11
#define SYNC_TRANSFER_INTERVAL_MS 100  // 每秒同步一次
#define DYNAMIC_TEX_SIZE 128 * 1024
namespace vkn {
	static VkFence fenceList[MAX_TRANSFER_SYNC_FENCES - 1];
	static VkCommandBuffer* commandList;
	static pmr::FrameAllocatorPool vkCpuBufferPool;
	static VmaAllocator vmaAllocator;
	void BufferWorker::InitVmaAllocator(VkInstance instance)
	{
		// 因为用volk库手动加载所有Vulkan函数了，所以这里要给VMA传递获取函数地址的方法，让VMA可以正确获取Vulkan函数
		VmaVulkanFunctions vmaVkFunctions = {};
		vmaVkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vmaVkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo vmaInfo = {};
		vmaInfo.vulkanApiVersion = VK_HEADER_VERSION_COMPLETE;
		vmaInfo.instance = instance;
		vmaInfo.physicalDevice = mDevice.GetPhysical();
		vmaInfo.device = mDevice.Ptr();
		vmaInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		// 如果不手动加载Vulkan函数，这里可以填NULL
		vmaInfo.pVulkanFunctions = &vmaVkFunctions;

		vmaCreateAllocator(&vmaInfo, &vmaAllocator);
		mTransferBuffer.CreateDynamicBuffers();
		mGraphicsBuffer.CreateDynamicBuffers();
	}
	void BufferWorker::InitCommandBuffers(uint32_t frames)
	{
		commandList = (VkCommandBuffer*)xmalloc(MAX_TRANSFER_SYNC_FENCES * (frames + 1) * sizeof(VkCommandBuffer));
		for (uint32_t i = 0; i <= frames; i++) {
			for (uint32_t j = 0; j < MAX_TRANSFER_SYNC_FENCES - 1; j++) {
				uint32_t k = j + (MAX_TRANSFER_SYNC_FENCES - 1) * i;
				if (i == frames) {
					fenceList[j] = mDevice.CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
					commandList[k] = GetCommandPool().AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
				}
				else {
					commandList[k] = Backend::RenderWorker->GetCommandPool().AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
				}
			}
		}
		vkResetFences(mDevice.Ptr(), MAX_TRANSFER_SYNC_FENCES - 1, fenceList);
		mTransferBuffer.pCommands = commandList + (MAX_TRANSFER_SYNC_FENCES - 1) * frames;
		mGraphicsBuffer.pCommands = commandList;
		mFrameNumber = frames;
	}
	void BufferWorker::TrySyncTransfer(VulkanContext& ctx)
	{
		uint32_t frameNumber = ctx.frameNumber++;
		if (frameNumber - mTransferBuffer.safeFrameNumber > SYNC_TRANSFER_INTERVAL_MS && mTransferBuffer.commandIndex < MAX_TRANSFER_SYNC_FENCES) {
			mTransferBuffer.commandIndex += MAX_TRANSFER_SYNC_FENCES;
			Invoke(SyncTransferCommand{ frameNumber });
		}
		mFrameNumber = frameNumber;//保守一点，慢一帧
		mGraphicsBuffer.commandIndex = 0;
		mGraphicsBuffer.pCommands = commandList + (MAX_TRANSFER_SYNC_FENCES - 1) * ctx.frame;
		mGraphicsBuffer.safeFrameNumber = ctx.frameNumber - ctx.frameCount;
	}
	void BufferWorker::SyncTransfer(const SyncTransferCommand& elem)
	{
		DynamicBufferPool& bufferPoolRef = mTransferBuffer;
		uint32_t index = bufferPoolRef.commandIndex % (MAX_TRANSFER_SYNC_FENCES);
		if (index) { //没有提交任务时，不需要等待
			vkWaitForFences(mDevice.Ptr(), index - 1, fenceList, VK_TRUE, UINT64_MAX);
			vkResetFences(mDevice.Ptr(), index - 1, fenceList);
		}
		bufferPoolRef.safeFrameNumber = elem.frameNumber;
		bufferPoolRef.commandIndex = 0;
	}
	void BufferWorker::Loop()
	{
		while (true) {
			Element elem = mChannel.acquire();
			std::visit(meta::overloaded{
			[&](BufferUpload& cmd) { UploadBuffer(cmd, TransferBufferType::Transfer); },
			[&](BufferCreator& cmd) { CreateBuffer(cmd); },
			[&](ImageUpdator& cmd) { UpdateImage(cmd); },
			[&](SyncTransferCommand& cmd) { SyncTransfer(cmd); },
				}, elem.cmd);
		}
	}
	void BufferWorker::UploadBuffer(const BufferUpload& elem, TransferBufferType type)
	{
		bool isTransfer = type == TransferBufferType::Transfer;
		DynamicBufferPool* pPoolRef;
		CommandBuffer command;
		if (!BeginDynamicCommand(pPoolRef, command, isTransfer)) {
			Invoke(elem);
			return;
		}
		// 将数据复制到 Staging Buffer
		DynamicBuffer* uploadBuffer = pPoolRef->FindUploadBuffer(elem.size, mFrameNumber + 1,elem.pCpuData);

		VmaAllocationCreateInfo allocationInfo = {};
		allocationInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.size = elem.size;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// 创建 GPU Buffer, GPU内部缓冲区，访问速度非常快
		VkBuffer& gpuBuffer = *elem.pBuffer;
		bufferInfo.usage = elem.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocationInfo, &gpuBuffer, &*elem.pAllocation, nullptr);

		// 复制数据
		VkBufferCopy copyRegion = {};
		copyRegion.size = elem.size;
		copyRegion.srcOffset = uploadBuffer->drawPos;
		vkCmdCopyBuffer(command.Ptr(), (VkBuffer)uploadBuffer->currentPage->pBuffer, gpuBuffer, 1, &copyRegion);

		EndDynamicCommand(pPoolRef, command, isTransfer);
	}
	void BufferWorker::CreateBuffer(BufferCreator& elem)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = elem.size;
		bufferInfo.usage = elem.usage;

		VmaAllocationCreateInfo allocationInfo = {};
		allocationInfo.usage = elem.memoryUsage;
		allocationInfo.requiredFlags = elem.momoryFlags;
		if (elem.ppCpuData)
			allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocationInfo, elem.pBuffer, elem.pAllocation, nullptr);
		if (elem.ppCpuData)
			vmaMapMemory(vmaAllocator, *elem.pAllocation, elem.ppCpuData);
	}
	void BufferWorker::CreateImage(ImageCreator& elem)
	{
		// 创建图像并分配内存
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VmaAllocation allocation;
		V(vmaCreateImage(vmaAllocator, &elem.imageInfo, &allocCreateInfo, elem.image, &allocation, nullptr));
		zlog::info("CreateImage {:#x} {} {}::{}",(uintptr_t)*elem.image, (uint32_t)elem.imageInfo.format, elem.imageInfo.extent.width, elem.imageInfo.extent.height);
	}
	void BufferWorker::UpdateImage(ImageUpdator& elem, TransferBufferType type)
	{
		bool isTransfer = type == TransferBufferType::Transfer;
		DynamicBufferPool* pPoolRef;
		CommandBuffer command = elem.command;
		if (!BeginDynamicCommand(pPoolRef, command, isTransfer)) {
			Invoke(elem);
			return;
		}
		TextureUpdateArgs args = elem.args;
		// 将数据复制到 Staging Buffer
		DynamicBuffer* uploadBuffer = pPoolRef->FindUploadBuffer(elem.size, mFrameNumber + 1, args.data);
		zlog::info("update {:#x} {}::{} {}::{}", (uintptr_t)elem.image,args.x,args.y ,args.width, args.height);
		VkBufferImageCopy region{};
		region.imageOffset.x = args.x;
		region.imageOffset.y = args.y;
		region.imageExtent.width = args.width;
		region.imageExtent.height = args.height;
		region.imageExtent.depth = 1;
		region.imageSubresource.mipLevel = args.mipLevel;
		region.bufferOffset = uploadBuffer->drawPos;
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		vkCmdCopyBufferToImage(command.Ptr(), (VkBuffer)uploadBuffer->currentPage->pBuffer, elem.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		if(!elem.command)
			EndDynamicCommand(pPoolRef, command, isTransfer);
	}
	bool BufferWorker::BeginDynamicCommand(DynamicBufferPool*& pPoolRef, CommandBuffer& command, bool isTransfer)
	{
		pPoolRef = isTransfer ? &mTransferBuffer : &mGraphicsBuffer;
		if (command.Ptr()) {
			return true;
		}
		if (pPoolRef->commandIndex >= MAX_TRANSFER_SYNC_FENCES) {
			return false;
		}
		command = pPoolRef->pCommands[pPoolRef->commandIndex];
		command.BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return true;
	}
	void BufferWorker::EndDynamicCommand(DynamicBufferPool* pPoolRef, CommandBuffer command, bool isTransfer)
	{
		command.EndRecord();
		pPoolRef->commandIndex++;
		if (isTransfer) {
			command.Submit(mQueue.Ptr(), fenceList[pPoolRef->commandIndex - 1]);
			if (pPoolRef->commandIndex >= MAX_TRANSFER_SYNC_FENCES) {
				SyncTransfer(SyncTransferCommand{ mFrameNumber });
			}
			return;
		}
		command.Submit(Backend::RenderWorker->GetQueue().Ptr(), nullptr);
	}
	void DynamicBufferPool::CreateDynamicBuffers()
	{
		uploadBuffer.InitBuffer(DYNAMIC_TEX_SIZE, BufferUsage::TRANSFER_SRC, 
			ResourceMemoryUsage::HOST_VISIBLE | ResourceMemoryUsage::HOST_COHERENT);
	}
	DynamicBuffer* DynamicBufferPool::FindUploadBuffer(uint32_t size, uint32_t frameNumber, const void* data)
	{
		// 将数据复制到 Staging Buffer
		void* pMappingAddr = uploadBuffer.MapBuffer(meta_align_size(size, 4), frameNumber, safeFrameNumber);
		if (data) {
			memcpy(pMappingAddr, data, size);
		}
		return &uploadBuffer;
	}
}
