#pragma once
#include "render/graph/frame_graph.h"
#include <imgui.h>
namespace api {
    class EditorSystem;
    struct RenderEditorContext {
        EditorSystem* editor;
        uint32_t frame;
        uint32_t frameCount;
        EditorSystem* operator->() {
            return editor;
        }
    };
    class EditorPanel {
    public:
        EditorPanel() = default;
        ~EditorPanel() = default;
        virtual void DrawPanel(FrameGraph& graph, RenderEditorContext& ctx) = 0;
    };
    class EditorWindow {
    protected:
        std::vector<EditorPanel*> mPanels;
    public:
        EditorWindow() = default;
        ~EditorWindow() = default;
        virtual void Draw(FrameGraph& graph, RenderEditorContext& ctx) = 0;
        template<typename T, typename ... Args>
        T* AddPanel(Args&&... args) {
            T* ptr = new (GlobalPool()) T(std::forward<Args>(args)...);
            mPanels.push_back(ptr);
            return ptr;
        }
    };
    class EDITOR_API EditorSystem : public ISystem
    {
        SINGLETON_IMPL(EditorSystem)
    protected:
        std::vector<EditorWindow*> mWindows;
    public:
        EditorSystem();
        template<typename T, typename ... Args>
        T* AddWindow(Args&&... args) {
            T* ptr = new (GlobalPool()) T(std::forward<Args>(args)...);
            mWindows.push_back(ptr);
            return ptr;
        }
        virtual void Render(FrameGraph& graph, RenderPassContext& ctx) = 0;
        ImTextureID  AddTexture(FrameGraph& graph, TextureDesc& desc, TextureSampler sampler);
        virtual ImTextureID AddTexture(ImageViewPtr imageview, SamplerPtr sampler, ResourceState state) = 0;
    };
}