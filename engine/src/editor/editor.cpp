#include "editor/editor.h"
#include "editor/window/editor_main_window.h"
#include "os/file_manager.h"
#include "imgui.h"
namespace api {
	void EditorModule::OnLoad(int argc, char** argv)
	{
		PackagePath editor_noesis{"/engine/assets/noesis"};
		FileManager::Ptr()->Mount("editor_noesis", editor_noesis.RealPath().c_str());
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 可选：启用键盘导航
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // 可选：启用Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // 可选：启用多视口
		ImGui::StyleColorsDark();
	}

	void EditorModule::OnUnload()
	{
	}
	void EditorModule::Initialize(void)
	{
		EditorSystem* Editor = EditorSystem::Ptr();
		Editor->AddWindow<EditorMainWindow>();
	}
}