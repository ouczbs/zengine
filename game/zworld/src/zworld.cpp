#include "zlog.h"
#include "zworld.h"
#include "data/global.h"
#include "event/event_system.h"
#include "vkn/vulkan_window.h"
#include "vkn/vulkan_api.h"
#include "render/pass/demo_pass.h"
#ifdef WITH_EDITOR
    #include "imgui/imgui_impl_sdl2.h"
#endif
#include <iostream>
using namespace api;
RenderAPI* API;
void ZWorldModule::OnLoad(int argc, char** argv)
{
    using vkn::VulkanWindow;
    using vkn::VulkanAPI;
    // 创建窗口
    gEngineConfig.API = GraphicsAPI::Vulkan;
    VulkanWindow* window = new VulkanWindow({ "zengine" , SDL_WINDOW_VULKAN }, 1080, 720);
    API = new vkn::VulkanAPI(window);
    auto args = vkn::VulkanWindowArgs::Default(3);
    window->CreateRender(args);
    API->Init();
    API->context.views.push_back({});
#ifdef WITH_EDITOR //绑定窗口交互
    ImGui_ImplSDL2_InitForVulkan(window->GetPtr());
#endif
}
void ZWorldModule::InitMetaData()
{
    mInfo.name = "zworld";
	mInfo.dependencies =  {
 		{"engine", "1.0.1", "shared" },
		{"editor", "1.0.1", "shared" },
		{"vulkan", "1.0.1", "shared" }
	};
}
void ZWorldModule::Initialize()
{
    EventSystem::Ptr()->BeginRenderFrame.Subscribe(mInfo.name, [](FrameGraph& graph, uint32_t frame) {
        graph.AddRenderPass<DemoPass>();
    });
}
void ZWorldModule::OnUnload()
{
}

void ZWorldModule::MainLoop()
{
    bool running = true;
    SDL_Event event_;
    while (running) {
        // 处理事件
        while (SDL_PollEvent(&event_)) {
#ifdef WITH_EDITOR
            ImGui_ImplSDL2_ProcessEvent(&event_);
#endif // WITH_EDITOR
            if (event_.type == SDL_QUIT) {
                running = false;
            }
        }  
        API->BeginFrame();
        API->Render();
        API->EndFrame();
        FramePool()->reset();
    }
}