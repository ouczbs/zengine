#include "vkn/wrapper/instance_create.h"
#include "vkn/wrapper/instance.h"
#include "zlog.h"
namespace vkn {
	VkDebugUtilsMessengerCreateInfoEXT InstanceCreator::DebugUtilsLayerNext()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		return createInfo;
	}
	InstanceCreator::InstanceCreator()
		: desiredExtensions(FramePool()), desiredLayers(FramePool())
		, appName("Vulkan Application")
		, appVersion(VK_API_VERSION_1_3)
		, engineName("zengine")
		, engineVersion(VK_API_VERSION_1_3)
		, apiVersion(VK_API_VERSION_1_3)
#ifdef Z_RENDER_DEBUG
		, messageSeverity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		, messageType(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		, debugCallback(Instance::DebugCallback)
#endif
	{

	}
	void InstanceCreator::AddExtension(string_view extensionName)
	{
		desiredExtensions.push_back(pmr::string{ extensionName, FramePool() });
	}
	void InstanceCreator::AddLayer(string_view layerName)
	{
		desiredLayers.push_back(pmr::string{ layerName, FramePool() });
	}
	void InstanceCreator::AddWindowExtension()
	{
#if defined(__ANDROID__)
		AddExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
		AddExtension(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11)
	#if defined(FILAMENT_SUPPORTS_XCB)
		AddExtension(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	#endif
	#if defined(FILAMENT_SUPPORTS_XLIB)
		AddExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
	#endif
#elif defined(WIN32)
		AddExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
	}
	void InstanceCreator::AddInstanceExtension()
	{
		AddExtension(VK_KHR_SURFACE_EXTENSION_NAME);
		AddExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		AddExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	}
#ifdef Z_RENDER_DEBUG
	void InstanceCreator::AddDebugExtension()
	{
		AddLayer("VK_LAYER_KHRONOS_validation");
		AddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		AddExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
#endif // Z_RENDER_DEBUG
	pmr::vector<char const*> InstanceCreator::EnabledExtensionNames()
	{
        pmr::vector<VkExtensionProperties> available_extensions{ FramePool() };
        pmr::vector<char const*> _extension{ FramePool() };
		if (!CheckAvailableInstanceExtensions(available_extensions)) {
			return _extension;
		}
		for (int i = 0, l = desiredExtensions.size(); i < l; i++) {
			if (IsExtensionSupported(available_extensions, desiredExtensions[i].c_str())) {
				_extension.push_back(desiredExtensions[i].c_str());
			}
			else {
				zlog::error("cann't support extension: {}", desiredExtensions[i].c_str());
			}
		}
        return _extension;
	}
	pmr::vector<char const*> InstanceCreator::EnabledLayerNames()
	{
        pmr::vector<VkLayerProperties> available_layers{ FramePool() };
        pmr::vector<char const*> _layers{ FramePool() };
		if (!CheckAvailableInstanceLayers(available_layers)) {
			return _layers;
		}
		for (int i = 0, l = desiredLayers.size(); i < l; i++) {
			if (IsLayerSupported(available_layers, desiredLayers[i].c_str())) {
				_layers.push_back(desiredLayers[i].c_str());
			}
			else {
				zlog::error("Could not load instance-level Vulkan function named: {}", desiredLayers[i].c_str());
			}
		}
        return _layers;
	}
	bool InstanceCreator::CheckAvailableInstanceExtensions(pmr::vector<VkExtensionProperties>& available_extensions)
	{
		uint32_t extensions_count = 0;
		VkResult result = VK_SUCCESS;

		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
		if ((result != VK_SUCCESS) ||
			(extensions_count == 0)) {
			zlog::error("Could not get the number of instance extensions.");
			return false;
		}

		available_extensions.resize(extensions_count);
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, available_extensions.data());
		if ((result != VK_SUCCESS) ||
			(extensions_count == 0)) {
			zlog::error("Could not enumerate instance extensions.");
			return false;
		}
		return true;
	}
	bool InstanceCreator::CheckAvailableInstanceLayers(pmr::vector<VkLayerProperties>& available_layers)
	{
		uint32_t extensions_count = 0;
		VkResult result = VK_SUCCESS;

		result = vkEnumerateInstanceLayerProperties(&extensions_count, nullptr);
		if ((result != VK_SUCCESS) ||
			(extensions_count == 0)) {
			zlog::error("Could not get the number of instance layers.");
			return false;
		}

		available_layers.resize(extensions_count);
		result = vkEnumerateInstanceLayerProperties(&extensions_count, available_layers.data());
		if ((result != VK_SUCCESS) ||
			(extensions_count == 0)) {
			zlog::error("Could not enumerate instance layers.");
			return false;
		}

		return true;
	}
	bool InstanceCreator::IsExtensionSupported(pmr::vector<VkExtensionProperties> const& available_extensions, char const* const extension)
	{
		for (auto& available_extension : available_extensions) {
			if (strstr(available_extension.extensionName, extension)) {
				return true;
			}
		}
		return false;
	}
	bool InstanceCreator::IsLayerSupported(pmr::vector<VkLayerProperties> const& available_layers, char const* const layer)
	{
		for (auto& available_layer : available_layers) {
			if (strstr(available_layer.layerName, layer)) {
				return true;
			}
		}
		return false;
	}
}
