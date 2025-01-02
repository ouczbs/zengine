#pragma once
#include "render/render_api.h"
#include "render/render_buffer.h"
#include <NsRender/RenderDevice.h>
#include <NsRender/Texture.h>
namespace api {
    class UITexture final : public Noesis::Texture
    {
    public:
        UITexture(TextureDesc desc) : desc(desc) {}
        ~UITexture()
        {

        }
        uint32_t GetWidth() const override { return desc.width; }
        uint32_t GetHeight() const override { return desc.height; }
        bool HasMipMaps() const override { return desc.mipLevel > 1; }
        bool IsInverted() const override { return isInverted; }
        bool HasAlpha() const override { return hasAlpha; }

        TextureDesc         desc;
        ImageViewPtr        imageView;
        bool isInverted = false;
        bool hasAlpha = false;
    };
    struct UIRenderBuffer {
        DynamicBuffer		mVertices;
        DynamicBuffer		mIndices;
        DynamicBuffer		mConstants[4];
    };
    class UI_API UIRenderSystem : public ISystem
    {
        SINGLETON_IMPL(UIRenderSystem)
    protected:
        UIRenderBuffer* mRenderBuffer;
    public:
        UIRenderSystem();
        void Initialize() override;
        void Finalize() override;
        Noesis::RenderDevice* GetRenderDevice();
        bool IsValidShader(Noesis::Shader shader, Noesis::RenderState state, bool hasExtendedDynamicState);
        virtual void InitNoesisRender(bool linearRendering, bool stereoSupport, SampleCount sampleCount) = 0;
        virtual void BeginRender(RenderContext* context) = 0;
        virtual void DrawBatch(const Noesis::Batch& batch) = 0;
    };
}