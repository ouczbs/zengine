#pragma once
#include "package_path.h"
namespace api
{
    class FileManagerImpl;
    class CORE_API FileManager
    {
        FileManagerImpl* impl;
        SINGLETON_IMPL(FileManager)
    public:
        FileManager();
        ~FileManager();
    public:
        void Mount(Name name, Name path);
        string_view FindMountPath(Name id);
        uint32_t FindPathFlag(PackagePath pack_path);
        FileBlock* FindPathBlock(PackagePath pack_path);
        void SaveMountMap();
        void LoadFileMap();
        void SaveFileMap();
        pmr::string RealPath(PackagePath pack_path);
    };
}
