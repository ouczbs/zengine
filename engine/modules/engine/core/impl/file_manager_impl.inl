#include "os/file_manager.h"
#include "os/file_system.h"
#include "os/file_handle.h"
#include "archive/pch.h"
#include "data/global.h"
#include <fstream>
namespace gen {
    template<>
    struct JsonSerde<api::FileBlock> {
        using T = api::FileBlock;
        inline static bool Read(yyjson_val* val, const void* ptr) {
            T* v = (T*)ptr;
            JsonRead(yyjson_obj_get(val, "path"), v->path);
            JsonRead(yyjson_obj_get(val, "addr"), v->addr);
            JsonRead(yyjson_obj_get(val, "flag"), v->flag);
            return true;
        }
        inline static yyjson_mut_val* Write(yyjson_mut_doc* doc, const void* ptr) {
            T* v = (T*)ptr;
            yyjson_mut_val* obj = yyjson_mut_obj(doc);
            yyjson_mut_obj_add_val(doc, obj, "path", JsonWrite(doc, v->path));
            yyjson_mut_obj_add_val(doc, obj, "addr", JsonWrite(doc, v->addr));
            yyjson_mut_obj_add_val(doc, obj, "flag", JsonWrite(doc, v->flag));
            return obj;
        }
    };
}
namespace api {
    using pmr::table;
    SINGLETON_DEFINE(FileManager)
    class FileManagerImpl
    {
    public:
        FileManagerImpl();
        ~FileManagerImpl();
    public:
        void Mount(Name name, Name path) {
            MountMap.emplace(name, path);
        }
        string_view FindMountPath(Name id) {
            auto it = MountMap.find(id);
            if (it != MountMap.end()) {
                return it->second.ToStringView();
            }
            return "";
        }
        uint32_t FindPathFlag(PackagePath pack_path) {
            auto it = FileMap.find(pack_path());
            if (it == FileMap.end()) {
                return FileFlag::File_Not_Exist;
            }
            return it->second.flag;
        }
        FileBlock* FindPathBlock(PackagePath pack_path) {
            auto it = FileMap.find(pack_path());
            if (it == FileMap.end()) {
                return nullptr;
            }
            return &it->second;
        }
        void SaveMountMap();
        void LoadFileMap();
        void SaveFileMap();
    private:
        table<Name, Name> MountMap;
        table<Name, FileBlock> FileMap;
    public:
        //外界不应该使用绝对路径
        pmr::string RealPath(PackagePath pack_path);
    };
    FileManagerImpl::FileManagerImpl()
	{
		Mount("exe", fs::GetExecutablePath());
		Mount("work", fs::GetWorkPath());
	}
    FileManagerImpl::~FileManagerImpl()
	{
		SaveMountMap();
		SaveFileMap();
	}
	void FileManagerImpl::SaveMountMap()
	{
        FileHandle handle(CFileMountName);
        handle.Open(FILE_OP::WRITE);
        handle.Write(JsonSerialize(MountMap));
	}
	void FileManagerImpl::LoadFileMap()
	{
        FileHandle handle(CFileMapName);
        handle.Open(FILE_OP::READ);
        if (handle) {
            pmr::string text = handle.ReadAll<pmr::string>();
            auto res = JsonDeserialize<table<Name, FileBlock>>(text);
            if (res) {
                FileMap = *res;
            }
        }
	}
	void FileManagerImpl::SaveFileMap()
	{
        FileHandle handle(CFileMapName);
        handle.Open(FILE_OP::WRITE);
        handle.Write(JsonSerialize(FileMap));
	}
	pmr::string FileManagerImpl::RealPath(PackagePath pack_path)
	{
		string_view name = pack_path.GetPackage();
		string_view pre_path = FindMountPath(name);
		if (name.empty() || pre_path.empty()) {
			return pmr::string(pack_path());
		}
		pmr::string path{FramePool()};
		path.reserve(pre_path.size() + pack_path.size() - name.size() - 1);
		path.append(pre_path);
		path.append(pack_path().substr(name.size() + 1));
		return path;
	}
    FileManager::FileManager()
    {
        SINGLETON_PTR()
        impl = new(GlobalPool()) FileManagerImpl();
        impl->LoadFileMap();
    }
    FileManager::~FileManager()
    {
        impl->~FileManagerImpl();
    }
    inline void FileManager::Mount(Name name, Name path)
    {
        impl->Mount(name, path);
    }
    inline string_view FileManager::FindMountPath(Name id)
    {
        return impl->FindMountPath(id);
    }
    inline uint32_t FileManager::FindPathFlag(PackagePath pack_path)
    {
        return impl->FindPathFlag(pack_path);
    }
    inline FileBlock* FileManager::FindPathBlock(PackagePath pack_path)
    {
        return impl->FindPathBlock(pack_path);
    }
    void FileManager::SaveMountMap() {
        return impl->SaveMountMap();
    }

    inline void FileManager::LoadFileMap()
    {
        impl->LoadFileMap();
    }

    inline void FileManager::SaveFileMap()
    {
        impl->SaveFileMap();
    }

    inline pmr::string FileManager::RealPath(PackagePath pack_path)
    {
        return impl->RealPath(pack_path);
    }
}

