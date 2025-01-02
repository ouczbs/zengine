#include "editor/window/editor_main_window.h"
#include "event/event_system.h"
#include "ui/ui_render_system.h"
#include "data/global.h"
#include <NoesisPCH.h>
namespace api {
	using namespace Noesis;
	void EditorMainUIWindow::InitializeComponent()
	{
		for (int i = 0; i < 1000; i++) {
			int j = 1;
		}
	}
	EditorMainWindow::EditorMainWindow()
	{
	}
	void EditorMainWindow::DrawNoesisUI()
	{
		if (!mView.GetPtr()) {
			Ptr<FrameworkElement> xaml = GUI::LoadXaml<FrameworkElement>("/editor_noesis/Menu.xaml");
			mView = GUI::CreateView(xaml);
			mView->SetFlags(RenderFlags_PPAA | RenderFlags_LCD);
			mView->SetSize(1080, 720);
			mView->GetRenderer()->Init(UIRenderSystem::Ptr()->GetRenderDevice());
		}

		mView->Update(0.033);
		IRenderer* renderer = mView->GetRenderer();
		renderer->UpdateRenderTree();
		renderer->RenderOffscreen();
		renderer->Render();
	}
	TextureSampler sampler{
		.filterMag = SamplerMagFilter::LINEAR,
		.filterMin = SamplerMinFilter::LINEAR,
		.wrapS = SamplerWrapMode::CLAMP_TO_EDGE,
		.wrapT = SamplerWrapMode::CLAMP_TO_EDGE,
		.wrapR = SamplerWrapMode::CLAMP_TO_EDGE,
		.compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE,
		.compareFunc = SamplerCompareFunc::GE,
	};
	ImTextureID TextureIDList[10] = {};
	void InitRenderSurface(FrameGraph& graph, uint32_t frameCount) {
		static bool sInit = false;
		if (sInit)
			return;
		sInit = true;
		TextureDesc desc{};
		desc.width = 512;
		desc.height = 512;
		desc.format = TinyImageFormat_B8G8R8A8_SRGB;
		desc.state = ResourceState::UNDEFINED;
		desc.sampleCount = SampleCount::SAMPLE_COUNT_1;
		desc.arraySize = 1;
		desc.mipLevel = 1;
		desc.depth = 1;
		desc.dimension = TextureDimension::TEX_2D;
		desc.usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;

		for (uint32_t i = 0; i < frameCount; i++) {
			graph.SetResourceTexture(desc, FrameGraph::NameEditorSurface, i);
		}
	}
	void EditorMainWindow::Draw(FrameGraph& graph, RenderEditorContext& ctx)
	{
		static float my_float = 0.5f;
		TextureDesc renderSurface = graph.GetRenderSurface();
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowViewport(viewport->ID);
		// 设置窗口的大小为屏幕的分辨率
		ImGui::SetNextWindowSize(ImVec2(renderSurface.width, renderSurface.height));
		// 设置窗口位置为左上角
		ImGui::SetNextWindowPos(viewport->Pos);

		ImGui::Begin("MainWindow", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("This is some useful text.");
		ImGui::SliderFloat("float", &my_float, 0.0f, 1.0f);
		if (gEngineConfig.IsRenderEditorSurface) {
			TextureDesc surface = graph.GetSurface();
			if (!TextureIDList[ctx.frame]) {
				TextureIDList[ctx.frame] = ctx->AddTexture(graph, surface, sampler);
			}
			// 每帧渲染时都可以通过 ImTextureID 使用
			ImGui::Image(TextureIDList[ctx.frame], ImVec2(surface.width, surface.height));
		}
		else {
			gEngineConfig.IsRenderEditorSurface = true;
			InitRenderSurface(graph, ctx.frameCount);
		}
		ImGui::End();
		DrawNoesisUI();
	}
}