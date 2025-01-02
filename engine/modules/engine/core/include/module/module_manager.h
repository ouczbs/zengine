#pragma once
#include "module.h"
namespace api {
    struct ModuleManagerImpl;
    class CORE_API ModuleManager
    {
        ModuleManagerImpl* impl;
    public:
        static ModuleManager* Ptr();
    public:
        ModuleManager();
        ~ModuleManager();
        IModule* GetModule(Name name);
        void RegisterModule(Name name, IModule::CreatePFN fn);
        void CreateModule(Name name, bool shared);
        void DestroyModule(Name name);
        void InitModule(Name name);
        void ShutModule(Name name);
        void MakeGraph(Name name, bool shared, int argc, char** argv);
        void MainLoop();
        void DestroyGraph();
    protected:
        IModule* spawnDynamicModule(Name name, bool hotfix);
        IModule* spawnStaticModule(Name name);
    };
    template <typename ModuleClass>
    struct ModuleRegistrantImpl {
        ModuleRegistrantImpl(const char* InModuleName)
        {
            ModuleManager::Ptr()->RegisterModule(InModuleName, []()->IModule* {
                return new ModuleClass();
            });
        }
    };
}