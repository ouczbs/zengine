#pragma once
#include "module/module.h"
namespace api {
    class EDITOR_API EditorModule : public IDynamicModule
    {
    public:
        void OnLoad(int argc, char** argv) override;
        void OnUnload() override;
        void InitMetaData(void) override {};
        void Initialize(void)override;
    };
    IMPLEMENT_DYNAMIC_MODULE(EDITOR_API, EditorModule, editor)
}