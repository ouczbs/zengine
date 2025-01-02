#pragma once
#include "vkn/type.h"
namespace vkn {
	class Instance;
	class DeviceCreator {
	private:
		class DesiredQueue final
		{
		public:
			Name name;
			VkQueueFlags flag;
			float prioritie;
			int queueFamilyIndex;
			DesiredQueue(Name name, VkQueueFlags flag, float prioritie)
				: name(name), flag(flag), prioritie(prioritie), queueFamilyIndex(0) {}
		};
	public:
		VkPhysicalDeviceFeatures desiredPhysicalDeviceFeatures;
		VkPhysicalDeviceType desiredPhysicalDeviceType;
		pmr::vector<pmr::string> desiredExtensions;
		pmr::vector<DesiredQueue> desiredQueues;
		int fencePoolCount = 256;
		Instance& instance;
	public:
		DeviceCreator(Instance& instance);

		void AddQueue(Name name, VkQueueFlags flag, float prioritie);
		void AddExtension(string_view extensionName);
		void AddWindowExtension();
		bool CheckProperty(const VkPhysicalDevice device);
		bool CheckExtension(const VkPhysicalDevice device);
		bool FindDevice(VkPhysicalDevice& device);
		void QueueCreateInfos(pmr::vector<VkDeviceQueueCreateInfo>& queue_create_infos,
			pmr::vector<pmr::vector<float>>& queue_prioritie,
			pmr::vector<VkQueueFamilyProperties>& queue_families);
		pmr::vector<char const*> EnabledExtensionNames();
		void EnableDeviceFeatures();
		VkPhysicalDeviceFeatures2 GetDeviceFeature2();
		VkPhysicalDeviceVulkan12Features GetVulkan12Features();
#ifdef Z_RENDER_DEBUG
	public:
		pmr::vector<pmr::string> desiredLayers;
		void AddLayer(string_view layerName);
		bool CheckLayer(const VkPhysicalDevice device);
		pmr::vector<char const*> EnabledLayerNames();
#endif
	public:
		static bool CheckAvailableQueueFamilies(VkPhysicalDevice physical_device, pmr::vector<VkQueueFamilyProperties>& queue_families);
	};
};