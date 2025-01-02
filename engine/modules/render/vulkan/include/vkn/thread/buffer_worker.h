#pragma once
#include "worker.h"
#include "vma/vk_mem_alloc.h"
#include "render/render_buffer.h"
#include <variant>
namespace vkn {
	class Device;
	class Queue;
	class VulkanContext;
	using  api::DynamicBuffer;
	extern pmr::FrameAllocatorPool vkCpuBufferPool;
	inline pmr::FrameAllocatorPool* BufferPool(){
		return &vkCpuBufferPool;
	}

	enum class TransferBufferType : uint8_t{
		Transfer,
		Graphics
	};
	struct Buffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
	};
	struct BufferUpload {
		VkBuffer* pBuffer;
		VmaAllocation* pAllocation;
		void* pCpuData;
		VkBufferUsageFlags usage;
		uint32_t size;
	};
	struct BufferCreator {
		VkBuffer*      pBuffer;
		VmaAllocation* pAllocation;
		void**		   ppCpuData;
		uint32_t       size;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags momoryFlags;
		VmaMemoryUsage memoryUsage;
	};
	struct ImageCreator {
		VkImageCreateInfo imageInfo;
		VkImage* image;
	};
	struct ImageUpdator {
		TextureUpdateArgs args;
		VkCommandBuffer command;
		VkImage        image;
		uint32_t       size;
	};
	struct SyncTransferCommand {
		uint32_t frameNumber;
	};
	struct BufferCommand {
		using Command = std::variant<BufferUpload, BufferCreator, ImageUpdator, SyncTransferCommand>;
		Command cmd;
		BufferCommand(const BufferUpload& cmd) : cmd(cmd) {}
		BufferCommand(const BufferCreator& cmd) : cmd(cmd) {}
		BufferCommand(const ImageUpdator& cmd) : cmd(cmd) {}
		BufferCommand(const SyncTransferCommand& cmd) : cmd(cmd) {}
	};
	struct DynamicBufferPool {
		TransferBufferType  type;
		uint32_t			commandIndex{0};
		uint32_t			safeFrameNumber{0};
		VkCommandBuffer*	pCommands;
		DynamicBuffer		uploadBuffer;
		DynamicBufferPool(TransferBufferType type) : type(type) {}
		void CreateDynamicBuffers();
		DynamicBuffer* FindUploadBuffer(uint32_t size,uint32_t frameNumber,const void* data);
	};
	class BufferWorker : public ThreadWorker<BufferCommand, BufferWorker>{
	public:
		uint32_t		  mFrameNumber{ 0 };
		DynamicBufferPool mTransferBuffer{ TransferBufferType::Transfer };
		DynamicBufferPool mGraphicsBuffer{ TransferBufferType::Graphics };
		using ThreadWorker<BufferCommand, BufferWorker>::ThreadWorker;
		void InitCommandBuffers(uint32_t frames);
		void InitVmaAllocator(VkInstance instance);
		void TrySyncTransfer(VulkanContext& ctx);
		void Loop();
		void SyncTransfer(const SyncTransferCommand& elem);
		void UploadBuffer(const BufferUpload& elem, TransferBufferType type = TransferBufferType::Graphics);
		void CreateBuffer(BufferCreator& elem);
		void CreateImage(ImageCreator& elem);
		void UpdateImage(ImageUpdator& elem, TransferBufferType type = TransferBufferType::Graphics);

		bool BeginDynamicCommand(DynamicBufferPool*& poolRef, CommandBuffer& cmd, bool isTransfer);
		void EndDynamicCommand(DynamicBufferPool* poolRef, CommandBuffer cmd, bool isTransfer);
	};
};
