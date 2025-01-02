#pragma once
#include "pmr/name.h"
#include "os/shared_library.h"
#include "refl/pch.h"
// 默认对齐方式 new
void* operator new(std::size_t size);
// 默认对齐方式 delete
void operator delete(void* ptr) noexcept;
// 自定义对齐 align
void* operator new(std::size_t size, std::align_val_t align);
namespace api {
    using pmr::Name;
    enum class EModuleFlag : uint32_t {
        Reload = 1,
    };
    struct ModuleDependency {
        UPROPERTY()
        Name name;
        UPROPERTY()
        Name version;
        UPROPERTY()
        Name kind;
    };
    struct ModuleInfo {
        UPROPERTY()
        EModuleFlag flag{0};
        UPROPERTY()
        Name name;         //!< name of the plugin
        UPROPERTY()
        Name prettyname;   //!< formatted name of the plugin
        UPROPERTY()
        Name core_version; //!< version of the engine
        UPROPERTY()
        Name version;      // !< version of the plugin
        UPROPERTY()
        Name linking;      // !< linking of the plugin
        UPROPERTY()
        Name license;      //!< license of the plugin
        UPROPERTY()
        Name url;          //!< url of the plugin
        UPROPERTY()
        Name copyright;    //!< copyright of the plugin
        UPROPERTY()
        pmr::vector<ModuleDependency> dependencies;
    public:
        bool IsReload() {
            return !!((uint32_t)flag & (uint32_t)EModuleFlag::Reload);
        }
    };
    struct ISubSystem
    {
        using CreatePFN = ISubSystem* (*)();
        virtual ~ISubSystem() = default;
        virtual void Initialize() = 0;
        virtual void Finalize() = 0;
    };
    struct IModule {
    public:
        friend class ModuleManagerImpl;
        using CreatePFN = IModule * (*)();
        IModule() = default;
        IModule(const IModule& rhs) = delete;
        IModule& operator=(const IModule& rhs) = delete;
        virtual ~IModule();
        virtual void OnLoad(int argc, char** argv) = 0;
        virtual void OnUnload() = 0;
        virtual void InitMetaData(void) = 0;
        virtual void Initialize();
        virtual void Finalize();
        template<typename T, typename ... Args>
        T* AddSystem(Args&&... args) {
            T* ptr = new (GlobalPool()) T(std::forward<Args>(args)...);
            mSystems.push_back(ptr);
            return ptr;
        }
        const ModuleInfo* GetModuleInfo()
        {
            return &mInfo;
        }

    protected:
        ModuleInfo mInfo;
        pmr::vector<ISubSystem*> mSystems;
    };
}
#include "module.inl"
#include ".core/module_gen.inl"