#pragma once
#include "module/module_manager.h"
namespace api {
    class RENDER_API RenderModule : public IStaticModule
    {
    public:
        void OnLoad(int argc, char** argv) override;
        void OnUnload() override;
        void InitMetaData(void) override {};
    };
}