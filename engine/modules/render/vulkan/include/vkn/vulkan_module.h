#pragma once
#include "module/module.h"
class VULKAN_API VulkanModule : public api::IDynamicModule
{
public:
    void OnLoad(int argc, char** argv) override;
    void OnUnload() override;
    void InitMetaData(void) override;
};
IMPLEMENT_DYNAMIC_MODULE(VULKAN_API, VulkanModule, vulkan)