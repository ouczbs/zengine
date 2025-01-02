#pragma once
#include <NsGui/TextureProvider.h>
namespace api {
    class UI_API NoesisTextureProvider : public Noesis::TextureProvider
    {
    public:
        NoesisTextureProvider();
        ~NoesisTextureProvider();

    private:
        /// From TextureProvider
        //@{
        Noesis::TextureInfo GetTextureInfo(const Noesis::Uri& uri) override;
        Noesis::Ptr<Noesis::Texture> LoadTexture(const Noesis::Uri& uri, Noesis::RenderDevice* device) override;
    };
}