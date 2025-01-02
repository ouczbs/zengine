#include "vkn/wrapper/instance.h"
#include "vkn/wrapper/instance_create.h"
#include "zlog.h"
namespace vkn {
	Instance::Instance(InstanceCreator& Creator)
	{
		V(volkInitialize());
		VkApplicationInfo application_info = {
		  VK_STRUCTURE_TYPE_APPLICATION_INFO,         // VkStructureType           sType
		  nullptr,                                    // const void              * pNext
		  Creator.appName.c_str(),                    // const char              * pApplicationName
		  Creator.appVersion,                         // uint32_t                  applicationVersion
		  Creator.engineName.c_str(),                 // const char              * pEngineName
		  Creator.engineVersion,                      // uint32_t                  engineVersion
		  Creator.apiVersion                          // uint32_t                  apiVersion
		};
#ifdef Z_RENDER_DEBUG
		Creator.AddDebugExtension();
#endif // Z_RENDER_DEBUG
		Creator.AddWindowExtension();
		Creator.AddInstanceExtension();
		auto extensions = Creator.EnabledExtensionNames();
		auto layers = Creator.EnabledLayerNames();;
		VkInstanceCreateInfo instance_create_info = {
		  VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,           // VkStructureType           sType
		  nullptr,                                          // const void              * pNext
		  0,                                                // VkInstanceCreateFlags     flags
		  &application_info,                                // const VkApplicationInfo * pApplicationInfo
		  static_cast<uint32_t>(layers.size()),             // uint32_t                  enabledLayerCount
		  layers.data(),                                    // const char * const      * ppEnabledLayerNames
		  static_cast<uint32_t>(extensions.size()),         // uint32_t                  enabledExtensionCount
		  extensions.data()                                 // const char * const      * ppEnabledExtensionNames
		};
#ifdef Z_RENDER_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugInfo = Creator.DebugUtilsLayerNext();
		instance_create_info.pNext = &debugInfo;
#endif
		V(vkCreateInstance(&instance_create_info, nullptr, &mPtr));
		volkLoadInstanceOnly(mPtr);
	}
	bool Instance::EnumerateAvailablePhysicalDevices(pmr::vector<VkPhysicalDevice>& available_devices)
	{
		uint32_t devices_count = 0;
		VkResult result = VK_SUCCESS;

		result = vkEnumeratePhysicalDevices(mPtr, &devices_count, nullptr);
		if ((result != VK_SUCCESS) ||
			(devices_count == 0)) {
			zlog::error("Could not get the number of available physical devices.");
			return false;
		}

		available_devices.resize(devices_count);
		result = vkEnumeratePhysicalDevices(mPtr, &devices_count, available_devices.data());
		if ((result != VK_SUCCESS) ||
			(devices_count == 0)) {
			zlog::error("Could not enumerate physical devices.");
			return false;
		}

		return true;
	}
	VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
	{
		zlog::error("validation layer: {}", pCallbackData->pMessage);
		return VK_FALSE;
	}
}
