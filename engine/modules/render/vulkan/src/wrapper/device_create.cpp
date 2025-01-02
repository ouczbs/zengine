#include "vkn/wrapper/device_create.h"
#include "vkn/wrapper/instance.h"
#include "zlog.h"
namespace vkn {
	DeviceCreator::DeviceCreator(Instance& instance)
		: instance(instance)
		, desiredPhysicalDeviceType(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		, desiredPhysicalDeviceFeatures()
		, desiredExtensions{FramePool()}
		, desiredQueues{ FramePool() }
#ifdef Z_RENDER_DEBUG
		, desiredLayers{FramePool()}
#endif
	{
	}
	void DeviceCreator::AddQueue(Name name, VkQueueFlags flag, float prioritie)
	{
		desiredQueues.emplace_back(name, flag, prioritie);
	}
	void DeviceCreator::AddExtension(string_view extensionName)
	{
		desiredExtensions.push_back(pmr::string{ extensionName , FramePool()});
	}
	void DeviceCreator::AddWindowExtension()
	{
		AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		//AddExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
#ifdef Z_RENDER_DEBUG
		AddLayer("VK_LAYER_KHRONOS_validation");
		AddLayer("VK_LAYER_RENDERDOC_Capture");
#endif
	}
	bool DeviceCreator::CheckProperty(const VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		return (deviceProperties.deviceType & desiredPhysicalDeviceType) == desiredPhysicalDeviceType;
	}
	bool DeviceCreator::CheckExtension(const VkPhysicalDevice device)
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		pmr::vector<VkExtensionProperties> availableExtensions{FramePool()};
		availableExtensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		for (const auto& desiredExtension : desiredExtensions)
		{
			bool find = false;
			for (const auto& availableExtension : availableExtensions)
			{
				if (strcmp(availableExtension.extensionName, desiredExtension.c_str()) == 0)
				{
					find = true;
					break;
				}
			}
			if (!find)return false;
		}
		return true;
	}
	bool DeviceCreator::FindDevice(VkPhysicalDevice& device)
	{
		pmr::vector<VkPhysicalDevice> available_devices{FramePool()};
		instance.EnumerateAvailablePhysicalDevices(available_devices);
		for (int i = 0, size = available_devices.size(); i < size; i++)
		{
			device = available_devices[i];
			if (!CheckProperty(device))continue;
			if (!CheckExtension(device))continue;
#ifdef Z_RENDER_DEBUG
			if (!CheckLayer(device))continue;
#endif // Z_RENDER_DEBUG
			return true;
		}
		return false;
	}
	void DeviceCreator::QueueCreateInfos(pmr::vector<VkDeviceQueueCreateInfo>& queue_create_infos, pmr::vector<pmr::vector<float>>& queue_prioritie, pmr::vector<VkQueueFamilyProperties>& queue_families)
	{
		uint32_t size = queue_families.size();
		for (uint32_t i = 0; i < size; i++) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = i;
			queueCreateInfo.queueCount = 0;
			queue_create_infos.push_back(queueCreateInfo);
			queue_prioritie.emplace_back(pmr::vector<float>{FramePool()});
		}
		uint32_t max_value = 1;
		for (auto& queue : desiredQueues) {
			uint32_t index = -1;
			bool bFind = false;
			for (uint32_t i = 0; i < size; i++) {
				auto family = queue_families[i];
				if ((family.queueFlags & queue.flag) == queue.flag) {
					index = i;
					if (queue_create_infos[i].queueCount < max_value) {
						bFind = true;
						break;
					}
				}
			}
			if (index != -1 && queue_create_infos[index].queueCount < queue_families[index].queueCount) {
				queue.queueFamilyIndex = index;
				queue_create_infos[index].queueCount++;
				queue_prioritie[index].push_back(queue.prioritie);
			}
			if (!bFind)
				max_value++;
		}
		auto it = queue_create_infos.begin();
		for (uint32_t i = 0; i < size; i++) {
			if (it->queueCount == 0) {
				it = queue_create_infos.erase(it);
			}
			else {
				it->pQueuePriorities = queue_prioritie[i].data();
				it++;
			}
		}
	}
	pmr::vector<char const*> DeviceCreator::EnabledExtensionNames()
	{
		pmr::vector<char const*> _extension;
		for (int i = 0, l = desiredExtensions.size(); i < l; i++) {
			_extension.push_back(desiredExtensions[i].c_str());
		}
		return _extension;
	}
	void DeviceCreator::EnableDeviceFeatures()
	{
		desiredPhysicalDeviceFeatures.independentBlend = true;
		desiredPhysicalDeviceFeatures.depthClamp = true;
		desiredPhysicalDeviceFeatures.geometryShader = VK_TRUE;
	}
	VkPhysicalDeviceFeatures2 DeviceCreator::GetDeviceFeature2()
	{
		// 明确设备要使用的功能特性
		VkPhysicalDeviceFeatures2 deviceFeatures = {};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		// 启用对各向异性采样的支持
		deviceFeatures.features.samplerAnisotropy = VK_TRUE;
		deviceFeatures.features.geometryShader = VK_TRUE;
		deviceFeatures.features.sampleRateShading = VK_TRUE;
		deviceFeatures.features.shaderInt64 = VK_TRUE;
		deviceFeatures.features.fillModeNonSolid = VK_TRUE; // 启用 fillModeNonSolid 特性
		return deviceFeatures;
	}
	VkPhysicalDeviceVulkan12Features DeviceCreator::GetVulkan12Features()
	{
		// 添加Vulkan 1.2的特性
		VkPhysicalDeviceVulkan12Features deviceVulkan12Features = {};
		deviceVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		// 启用对Device Address的支持
		deviceVulkan12Features.bufferDeviceAddress = VK_TRUE;
		deviceVulkan12Features.runtimeDescriptorArray = VK_TRUE;
		deviceVulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		deviceVulkan12Features.hostQueryReset = VK_TRUE;
		deviceVulkan12Features.pNext = nullptr;
		return deviceVulkan12Features;
	}
#ifdef Z_RENDER_DEBUG
	void DeviceCreator::AddLayer(string_view layerName)
	{
		desiredLayers.push_back(pmr::string{ layerName , FramePool()});
	}
	bool DeviceCreator::CheckLayer(const VkPhysicalDevice device)
	{
		uint32_t layerCount = 0;
		vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);
		pmr::vector<VkLayerProperties> availableLayers{FramePool()};
		availableLayers.resize(layerCount);
		vkEnumerateDeviceLayerProperties(device, &layerCount, availableLayers.data());
		for (const auto& desiredLayer : desiredLayers)
		{
			bool find = false;
			for (const auto& availableLayer : availableLayers)
			{
				if (strcmp(availableLayer.layerName, desiredLayer.c_str()) == 0)
				{
					find = true;
					break;
				}
			}
			if (!find)return false;
		}
		return true;
	}
	pmr::vector<char const*> DeviceCreator::EnabledLayerNames()
	{
		pmr::vector<char const*> _extension{FramePool()};
		for (int i = 0, l = desiredLayers.size(); i < l; i++) {
			_extension.push_back(desiredLayers[i].c_str());
		}
		return _extension;
	}
#endif // Z_RENDER_DEBUG
	bool DeviceCreator::CheckAvailableQueueFamilies(VkPhysicalDevice physical_device, pmr::vector<VkQueueFamilyProperties>& queue_families)
	{
		uint32_t queue_families_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);
		if (queue_families_count == 0) {
			zlog::error("Could not get the number of queue families.");
			return false;
		}
		queue_families.resize(queue_families_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families.data());
		if (queue_families_count == 0) {
			zlog::error("Could not acquire properties of queue families.");
			return false;
		}
		return true;
	}
}
