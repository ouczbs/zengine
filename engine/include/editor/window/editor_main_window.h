#pragma once
#include "editor/editor_system.h"
#include "ui/ui_window.h"
namespace api {
	class EditorMainUIWindow : public UIWindow {
	public:
		using UIWindow::UIWindow;
		void InitializeComponent() override;
		
	};
	class EditorMainWindow : public EditorWindow {
	public:
		Noesis::Ptr<Noesis::IView> mView;
	public:
		EditorMainWindow();
		void DrawNoesisUI();
		void Draw(FrameGraph& graph, RenderEditorContext& ctx) override;
	};
}