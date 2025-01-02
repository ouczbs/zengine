#pragma once
#include "vkn/type.h"

namespace vkn {
	class InstanceCreator;
	class Instance {
		friend class InstanceCreator;
	protected:
		VkInstance mPtr;
	public:
		Instance(InstanceCreator& Creator);

		VkInstance& Ptr() {
			return mPtr;
		}
		bool EnumerateAvailablePhysicalDevices(pmr::vector<VkPhysicalDevice>& available_devices);

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
			VkDebugUtilsMessageTypeFlagsEXT,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void*);
	};
};