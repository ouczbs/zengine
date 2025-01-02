namespace api {
    using ISystem = ISubSystem;
    template <typename ModuleClass>
    struct ModuleRegistrantImpl;
    struct IDynamicModule : public IModule {
        SharedLibrary mSharedLib{};
    };
    struct DefaultDynamicModule : public IDynamicModule {
        DefaultDynamicModule(Name name) : IDynamicModule() {};
        void OnLoad(int argc, char** argv) override {};
        void OnUnload() override {};
        void InitMetaData(void) override {};
    };
    using IStaticModule = IModule;
    struct IHotfixModule : public IDynamicModule {
        void* state = nullptr;
        //IHotfixModule(){ mInfo.flag |= EModuleFlag::Reload; }
        virtual void OnBeginLoad() = 0;
        virtual void OnEndLoad() = 0;
    };
    struct IMainModule : public IDynamicModule {
        virtual void MainLoop() = 0;
    };
    class CORE_API CoreModule : public IStaticModule
    {
    public:
        void OnLoad(int argc, char** argv) override;
        void OnUnload() override;
        void InitMetaData(void) override;
    };
}
#define IMPLEMENT_STATIC_MODULE(DLL_APL, ModuleImplClass, ModuleName) \
    DLL_APL inline const api::ModuleRegistrantImpl<ModuleImplClass> __RegisterModule__##ModuleName(#ModuleName);

#define IMPLEMENT_DYNAMIC_MODULE(DLL_APL, ModuleImplClass, ModuleName)  \
    using __##ModuleName##__module = ModuleImplClass;                   \
    extern "C" DLL_APL api::IModule* __newDynamicModule__##ModuleName() \
    {                                                                   \
        return new(GlobalPool()) ModuleImplClass();                                   \
    }