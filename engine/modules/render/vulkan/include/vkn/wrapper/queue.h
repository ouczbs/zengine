#pragma once
#include "vkn/type.h"
namespace vkn {
	class Device;
	class CommandBuffer;
	class Queue {
	protected:
		VkQueue mPtr;
		uint32_t mQueueFamilyIndex;
		const Name mName;
	public:
		Queue(Name name, uint32_t queueFamilyIndex, VkQueue queue);
		uint32_t QueueFamilyIndex()
		{
			return mQueueFamilyIndex;
		}
		VkQueue& Ptr() {
			return mPtr;
		}
	public:
		static const Name TransferQueue;
		static const Name RenderQueue;
	};
}