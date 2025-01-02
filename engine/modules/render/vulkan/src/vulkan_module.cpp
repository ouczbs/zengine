#include "xmalloc_new_delete.h"
#include "vkn/vulkan_module.h"
#include "vkn/loader/vulkan_glsl_loader.h"
#include "pmr/frame_allocator.h"
#include "vkn/vulkan_ui_system.h"
#ifdef WITH_EDITOR
#include "vkn/vulkan_imgui_editor.h"
#endif // WITH_EDITOR
using namespace vkn;
void VulkanModule::OnLoad(int argc, char** argv)
{
	VulkanGlslLoader::Init();
	AddSystem<VulkanUISystem>();
#ifdef WITH_EDITOR
	AddSystem<VulkanImguiEditor>();
#endif // WITH_EDITOR
}

void VulkanModule::OnUnload()
{
}
void VulkanModule::InitMetaData()
{
		mInfo.name = "vulkan";
	mInfo.dependencies =  {
 		{"engine", "1.0.1", "shared" }
	};
}