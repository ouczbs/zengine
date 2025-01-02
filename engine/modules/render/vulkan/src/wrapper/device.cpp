#include "vkn/wrapper/device.h"
#include "vkn/wrapper/device_create.h"
#include "vkn/wrapper/queue.h"
#include "zlog.h"
namespace vkn {
	Device::Device(DeviceCreator& Creator) : mQueueMap(GlobalPool())
	{
        //物理设备
        Creator.FindDevice(mPhysical);
        //队列信息
        pmr::vector<VkQueueFamilyProperties> queue_families{FramePool()};
        Creator.CheckAvailableQueueFamilies(mPhysical, queue_families);
        pmr::vector<VkDeviceQueueCreateInfo> queue_create_infos{ FramePool() };
        pmr::vector<pmr::vector<float>> queue_prioritie;
        Creator.QueueCreateInfos(queue_create_infos, queue_prioritie, queue_families);
        //扩展
        auto extensions = Creator.EnabledExtensionNames();
        //特性
        VkPhysicalDeviceFeatures2 deviceFeatures = Creator.GetDeviceFeature2();
        VkPhysicalDeviceVulkan12Features deviceVulkan12Features = Creator.GetVulkan12Features();
        deviceFeatures.pNext = &deviceVulkan12Features;
        VkDeviceCreateInfo device_create_info = {
          VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,               // VkStructureType                  sType
          &deviceFeatures,                                    // const void                     * pNext
          0,                                                  // VkDeviceCreateFlags              flags
          static_cast<uint32_t>(queue_create_infos.size()),   // uint32_t                         queueCreateInfoCount
          queue_create_infos.data(),                          // const VkDeviceQueueCreateInfo  * pQueueCreateInfos
          0,                                                  // uint32_t                         enabledLayerCount
          nullptr,                                            // const char * const             * ppEnabledLayerNames
          static_cast<uint32_t>(extensions.size()),           // uint32_t                         enabledExtensionCount
          extensions.data(),                                  // const char * const             * ppEnabledExtensionNames
          nullptr                                             // const VkPhysicalDeviceFeatures * pEnabledFeatures
        };
#ifdef Z_USE_GRAPHIC_DEBUG
        auto layers = Creator.EnabledLayerNames();
        device_create_info.enabledLayerCount = layers.size();
        device_create_info.ppEnabledLayerNames = layers.data();
#endif // Z_USE_GRAPHIC_DEBUG
        VkResult result = vkCreateDevice(mPhysical, &device_create_info, nullptr, &mPtr);
        if ((result != VK_SUCCESS) || (mPtr == VK_NULL_HANDLE)) {
            zlog::error("Could not create logical device. VkResult {}", (int)result);
        }
        volkLoadDevice(mPtr);
        for (auto& queue : Creator.desiredQueues) {
            Queue* gq = new (GlobalPool()) Queue(queue.name, queue.queueFamilyIndex, VK_NULL_HANDLE);
            vkGetDeviceQueue(mPtr, queue.queueFamilyIndex, 0, &(gq->Ptr()));
            mQueueMap.emplace(queue.name, gq);
        }
	}
	Device::~Device()
	{
        for (auto& queue : mQueueMap) {
            queue.second->~Queue();
        }
        mQueueMap.clear();
	}
    VkFence Device::CreateFence(VkFenceCreateFlags flags)
    {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // 创建时立刻设置为signaled状态(否则第一次永远等不到)
        fenceInfo.flags = flags;
        VkFence fence;
        VkResult result = vkCreateFence(mPtr, &fenceInfo, nullptr, &fence);
        return fence;
    }
    Queue* Device::GetQueue(Name name)
    {
        auto it = mQueueMap.find(name);
        if (it != mQueueMap.end()) {
            return it->second;
        }
        return nullptr;
    }
    VkFence Device::PopFence()
    {
        if (mFencePool.empty()) {
            return CreateFence(0);
        }
        std::lock_guard<std::mutex> lock(mFenceMutex);
        VkFence fence = mFencePool.back();
        mFencePool.pop_back();
        return fence;
    }
    void Device::PushWaitFence(VkFence fence)
    {
        vkWaitForFences(mPtr, 1, &fence, VK_TRUE, UINT64_MAX);
        vkResetFences(mPtr, 1, &fence);
        std::lock_guard<std::mutex> lock(mFenceMutex);
        mFencePool.push_back(fence);
    }
    VkSemaphore Device::CreateSemaphore()
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore semaphore;
        VkResult result = vkCreateSemaphore(mPtr, &semaphoreInfo, nullptr, &semaphore);
        return semaphore;
    }
    void Device::CreateSemaphores(vector<VkSemaphore>& list, int size)
    {
        list.reserve(size);
        for (int i = 0; i < size; i++) {
            list.push_back(CreateSemaphore());
        }
    }
    VkShaderModule Device::CreateShaderModule(const pmr::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        // 这里需要确保数据满足uint32_t的对齐要求,存储在vector中，默认分配器已经确保数据满足最差情况下的对齐要求
        createInfo.codeSize = code.size();
        // 转换为Vulkan要求的uint32_t指针
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule module;
        VkResult result = vkCreateShaderModule(mPtr, &createInfo, nullptr, &module);
        return module;
    }
    VkShaderModule Device::CreateShaderModule(const pmr::vector<uint32_t>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        //code size 是字节大小，需要 * sizeof
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        // 转换为Vulkan要求的uint32_t指针
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule module;
        VkResult result = vkCreateShaderModule(mPtr, &createInfo, nullptr, &module);
        return module;
    }
}
