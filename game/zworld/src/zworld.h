#pragma once
#include "module/module.h"
#include "module/module_manager.h"
class ZWORLD_API ZWorldModule : public api::IMainModule
{
public:
    void OnLoad(int argc, char** argv) override;
    void Initialize()override;
    void OnUnload() override;
    void InitMetaData(void) override;
    void MainLoop()override;
};
IMPLEMENT_DYNAMIC_MODULE(ZWORLD_API, ZWorldModule, zworld)