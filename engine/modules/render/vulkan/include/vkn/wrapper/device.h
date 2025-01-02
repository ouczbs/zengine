#pragma once
#include "vkn/type.h"
namespace vkn {
	class DeviceCreator;
	class Queue;
	class Device {
		friend class DeviceCreator;
	protected:
		VkDevice mPtr{ NULL };
		VkPhysicalDevice mPhysical{ NULL };
		table<Name, Queue*> mQueueMap;
		pmr::vector<VkFence> mFencePool;
		std::mutex mFenceMutex;
	public:
		VkDevice& Ptr() {
			return mPtr;
		}
		VkPhysicalDevice GetPhysical() {
			return mPhysical;
		}
	public:
		Device(DeviceCreator& Creator);
		~Device();

		VkFence CreateFence(VkFenceCreateFlags flags);
		Queue* GetQueue(Name name);
		VkFence PopFence();
		void PushWaitFence(VkFence fence);
		VkSemaphore CreateSemaphore();
		void CreateSemaphores(vector<VkSemaphore>& list, int size);
		VkShaderModule CreateShaderModule(const pmr::vector<char>& code);
		VkShaderModule CreateShaderModule(const pmr::vector<uint32_t>& code);
	};
};