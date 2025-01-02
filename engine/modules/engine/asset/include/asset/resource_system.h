#pragma once
#include "module/module.h"
#include "res/resource_config.h"
#include "res/resource_bundle.h"
#include "res/meta_bundle.h"
#include "os/package_path.h"
#include "meta/result.h"
#include <memory>
#include <optional>
namespace api {
    using std::array;
    using std::shared_ptr;
    enum class ResourceLoadError : char
    {
        ExtensionNotRegistered,
        FileDoesNotExist,
        FailedToLoadResource,
    };
    struct ResourceFileFlag{
        enum Value : uint32_t {
            File_Default = 0,
            File_Dirty = 1 << 0,
            File_Meta_Dirty = 1 << 1,
            File_Loaded = 1 << 2,
        };
        Value val{ File_Default };
        // 构造函数和操作符重载，使类实例像整数一样使用
        ResourceFileFlag(Value v = File_Default) : val(v) {}
        operator uint32_t() const { return val; }
        ResourceFileFlag& operator=(Value v) {
            val = v;
            return *this;
        }
    };
    class IFileLoader
    {
    protected:
        uint32_t mFileFlag = 0;
    public:
        void SetFileFlag(uint32_t flag) {
            mFileFlag = flag;
        }
        virtual ResourceBundle LoadFile(PackagePath handle, const MetaBundle& meta) = 0;
        virtual void SaveFile(PackagePath handle, const ResourceBundle& bundle) {};
        virtual ~IFileLoader() = default;
    };
    template<typename Res>
    using LoadResult = result<Res, ResourceLoadError>; 
    class ResourceSystemImpl;
    class ASSET_API ResourceSystem : public ISystem
    {
        SINGLETON_IMPL(ResourceSystem)
    public:
        struct ResourceFileBlock;
        template<typename R>
        struct ResourceBlock;
        template<typename R>
        using ResourceStorage = table<Guid, ResourceBlock<R>>;
        using GenericPtr = shared_ptr<void>;
    private:
        ResourceSystemImpl* impl;
    public:
        ResourceSystem();
        void Initialize() override;
        void Finalize() override;
    public:
        IFileLoader* GetLoader(Name extension);
        void RegisterLoader(Name ext, IFileLoader* loader);
        ResourceFileBlock& GetFileBlock(PackagePath path);
        FileBlock* GetResourceFile(const Guid& guid);
        ResourceBundle& Load(PackagePath path, bool reload_resource = true, int deep = 0);
        MetaBundle GetMeta(PackagePath path);
        MetaBundle GetVisitMeta(const ResourceBundle& bundle);
        void AddDirtyFile(Name path);
        void SaveMeta(PackagePath path, const MetaBundle& bundle);
        void SaveDirtyFile();
        void SaveFile(PackagePath path, const ResourceBundle& bundle);
        void* GetTable(size_t resourceid);
        uint32_t GetFileFlag(Name name);
        void SetFileFlag(Name name, uint32_t flag);

        template<typename Res> 
        Res* Get(const RscHandle<Res>& handle,bool sync = true);
        template<typename Res> 
        auto& GetTable();
        template<typename Res>
        ResourceBlock<Res>* GetBlock(const RscHandle<Res>& handle);
        template<typename Res>
        [[nodiscard]] RscHandle<Res> LoadFromMeta(const Guid& ,const SerializedMeta& meta);
        template<typename Res, typename ... Args> 
        [[nodiscard]] RscHandle<Res> LoadEmplaceResource(Args&& ... args) { return LoadEmplaceResource<Res>(Guid::Make(), args...);};
        template<typename Res, typename ... Args> 
        [[nodiscard]] RscHandle<Res> LoadEmplaceResource(Guid, Args&& ... args);
        template<typename FLoader, typename ... Args> 
        FLoader& RegisterLoader(Name ext, Args&& ... args);
        template<typename Res>  
        RscHandle<Res> Load(PackagePath path, bool reload_resource = true);
    };
    struct ResourceSystem::ResourceFileBlock
    {
        ResourceBundle bundle;
        Name     path;
        uint32_t flag{0};
        ResourceFileBlock& SetFlag(bool is, ResourceFileFlag _flag) {
            if (is) 
                flag |= _flag;
            else 
                flag &= ~_flag;
            return *this;
        }
        operator bool() {
            return IsLoaded();
        }
        ResourceFileBlock& Loaded(bool is) { return SetFlag(is, ResourceFileFlag::File_Loaded); }
        ResourceFileBlock& Dirty(bool is) { return SetFlag(is, ResourceFileFlag::File_Dirty); }
        ResourceFileBlock& MetaDirty(bool is) { return SetFlag(is, ResourceFileFlag::File_Meta_Dirty); }
        bool IsLoaded() { return flag & ResourceFileFlag::File_Loaded; }
        bool IsDirty() { return flag & ResourceFileFlag::File_Dirty; }
        bool IsMetaDirty() { return flag & ResourceFileFlag::File_Meta_Dirty; }
    };
    template<typename R>
    struct ResourceSystem::ResourceBlock
    {
        R* resource{nullptr};
        ResourceFileBlock* file{ nullptr };
        bool valid() const { return resource; }
    };
}
#include "resource_system.inl"