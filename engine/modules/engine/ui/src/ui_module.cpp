#include "zlog.h"
#include "ui/ui_module.h"
#include "ui/ui_window.h"
#include "ui/noesis_xaml_provider.h"
#include "ui/noesis_font_provider.h"
#include "ui/noesis_texture_provider.h"
#include <NsGui/IntegrationAPI.h>
#include <NsGui/FontProperties.h>
#include <NsCore/RegisterComponent.inl>
namespace api {
    using namespace Noesis;
	void UIModule::OnLoad(int argc, char** argv)
	{
        SetLogHandler([](const char*, uint32_t, uint32_t level, const char*, const char* msg)
        {
            switch (level) {
            case 0:
            case 1:
                zlog::debug("[NOESIS] {}\n", msg);
                break;
            case 2:
                zlog::info("[NOESIS] {}\n", msg);
                break;
            case 3:
                zlog::warn("[NOESIS] {}\n", msg);
                break;
            default:
                zlog::error("[NOESIS] {}\n", msg);
                break;
            }
        });
        // Sets the active license
        GUI::SetLicense(NS_LICENSE_NAME, NS_LICENSE_KEY);
        // Noesis initialization. This must be the first step before using any NoesisGUI functionality
        GUI::Init();
	}
	void UIModule::OnUnload()
	{
        // 清理资源提供者
        GUI::SetFontProvider(nullptr);
        GUI::SetTextureProvider(nullptr);
	}
    void UIModule::Initialize()
    {
        IStaticModule::Initialize();
        GUI::SetXamlProvider(new NoesisXamlProvider());
        GUI::SetFontProvider(new NoesisFontProvider());
        GUI::SetTextureProvider(new NoesisTextureProvider());
        RegisterComponent<api::UIWindow>();
        SetThemeProviders();
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void UIModule::SetThemeProviders()
    {
        const char* fonts[] = {"Arial"};

        GUI::SetFontFallbacks(fonts, NS_COUNTOF(fonts));
        GUI::SetFontDefaultProperties(15.0f, FontWeight_Normal, FontStretch_Normal, FontStyle_Normal);
    }
}