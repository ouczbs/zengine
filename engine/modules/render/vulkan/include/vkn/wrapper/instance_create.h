#pragma once
#include "vkn/type.h"
namespace vkn {
	class InstanceCreator {
	public:
		pmr::string appName;
		uint32_t appVersion;
		pmr::string engineName;
		uint32_t engineVersion;
		uint32_t apiVersion;

		pmr::vector<pmr::string> desiredExtensions;
		pmr::vector<pmr::string> desiredLayers;

#ifdef Z_RENDER_DEBUG
		VkDebugUtilsMessageSeverityFlagsEXT messageSeverity;
		VkDebugUtilsMessageTypeFlagsEXT messageType;
		PFN_vkDebugUtilsMessengerCallbackEXT debugCallback;

		VkDebugUtilsMessengerCreateInfoEXT DebugUtilsLayerNext();
#endif
	public:
		InstanceCreator();
		void AddExtension(string_view extensionName);
		void AddLayer(string_view layerName);

		void AddWindowExtension();
		void AddInstanceExtension();
		void AddDebugExtension();

		pmr::vector<char const*> EnabledExtensionNames();
		pmr::vector<char const*> EnabledLayerNames();

	private:
		static bool CheckAvailableInstanceExtensions(pmr::vector<VkExtensionProperties>& available_extensions);
		static bool CheckAvailableInstanceLayers(pmr::vector<VkLayerProperties>& available_layers);
		static bool IsExtensionSupported(pmr::vector<VkExtensionProperties> const& available_extensions,
			char const* const extension);
		static bool IsLayerSupported(pmr::vector<VkLayerProperties> const& available_layers,
			char const* const layer);
	};
};