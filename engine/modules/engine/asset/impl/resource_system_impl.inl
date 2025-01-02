#include "asset/asset_module.h"
#include "asset/resource_system.h"
#include "os/file_manager.h"
#include "os/file_handle.h"
#include "archive/pch.h"
#include "data/global.h"
namespace api {
	SINGLETON_DEFINE(ResourceSystem)
	using GenericPtr = ResourceSystem::GenericPtr;
	using ResourceFileBlock = ResourceSystem::ResourceFileBlock;
	class ResourceSystemImpl
	{
	public:
		ResourceSystem* owner;
		uint32_t mFileFlag;
		array<GenericPtr, ResourceCount>      mResourceTable;
		table<Guid, FileBlock*>				  mResourceFile;
		table<Name, IFileLoader*>			  mFileLoader;
		table<Name, uint32_t>				  mFileFlags;
		table<Name, ResourceFileBlock>		  mFileBlock;
		vector<ResourceFileBlock*>			  mDirtyBlock;
	public:
		ResourceSystemImpl(ResourceSystem* owner);
		void Initialize();
		void Finalize();

	public:
		IFileLoader* GetLoader(Name extension);
		ResourceFileBlock& GetFileBlock(PackagePath path);
		FileBlock* GetResourceFile(const Guid& guid);

		ResourceBundle& Load(PackagePath path, bool reload_resource = true, int deep = 0);
		MetaBundle GetMeta(PackagePath path);
		MetaBundle GetVisitMeta(const ResourceBundle& bundle);
		void AddDirtyFile(Name path);
		void SaveMeta(PackagePath path, const MetaBundle& bundle);
		void SaveDirtyFile();
		void SaveFile(PackagePath path, const ResourceBundle& bundle);
		void LoadFileFlags();
		void SaveFileFlags();
		void LoadResourceFile();
		void SaveResourceFile();
		uint32_t GetFileFlag(Name name);
		void SetFileFlag(Name name, uint32_t flag);
	};
	ResourceSystemImpl::ResourceSystemImpl(ResourceSystem* owner) : owner(owner)
	{
		mResourceTable = detail::ResourceHelper::GenResourceTables();
		LoadFileFlags();
		LoadResourceFile();
	}
	void ResourceSystemImpl::Initialize()
	{
		SaveFileFlags();
		SaveDirtyFile();
	}
	void ResourceSystemImpl::Finalize()
	{
		SaveFileFlags();
		SaveDirtyFile();
		SaveResourceFile();
	}
	IFileLoader* ResourceSystemImpl::GetLoader(Name extension)
	{
		auto itr = mFileLoader.find(extension);
		if (itr == mFileLoader.end())
			return nullptr;
		return itr->second;
	}
	ResourceBundle& ResourceSystemImpl::Load(PackagePath path, bool reload_resource, int deep)
	{
		Name name = path();
		auto it = mFileBlock.find(name);
		auto& res = mFileBlock[name];
		if (deep > 5 || (!reload_resource && res && it != mFileBlock.end())) {
			return res.bundle;
		}
		Name ext = path.GetExtension();
		IFileLoader* loader = GetLoader(ext);
		MetaBundle meta = GetMeta(path);
		ResourceBundle bundle = loader->LoadFile(path, meta);
		MetaBundle new_meta = GetVisitMeta(bundle);
		bool is_dirty_meta = meta != new_meta;
		for (auto& elem : bundle.GetAll()) {
			std::visit([&](auto& handle) {
				using T = std::decay_t<decltype(*handle)>;
				owner->GetBlock<T>(handle)->file = &res;
				mResourceFile[handle.guid] = FileManager::Ptr()->FindPathBlock(path);
			}, elem);
		}
		if (is_dirty_meta || res.IsDirty()) {
			mDirtyBlock.push_back(&res);
		}
		res.path = name;
		res.bundle = bundle;
		res.Loaded(true).MetaDirty(is_dirty_meta);
		res.bundle = bundle;
		if (!new_meta.includes.empty()) {
			for (auto include : new_meta.includes) {
				Load(include, false, deep + 1);
			}
		}
		return res.bundle;
	}

	MetaBundle ResourceSystemImpl::GetMeta(PackagePath path)
	{
		FileHandle handle(path + ".meta");
		if (!handle.Open(FILE_OP::READ, mFileFlag & FileFlag::File_Binary)) {
			return {};
		}
		meta::result<MetaBundle, SerializeError> res;
		if (mFileFlag & FileFlag::File_Binary) {

		}
		else {
			pmr::string text = handle.ReadAll<pmr::string>();
			res = TextDeserialize<MetaBundle>(text);
		}
		if (!res) {
			return {};
		}
		return res.value();
	}
	void ResourceSystemImpl::SaveMeta(PackagePath path, const MetaBundle& bundle)
	{
		FileHandle handle(path + ".meta");
		handle.Open(FILE_OP::WRITE, mFileFlag & FileFlag::File_Binary);
		if (mFileFlag & FileFlag::File_Binary) {
			
		}
		else {
			handle.Write(TextSerialize(bundle));
		}
	}
	void ResourceSystemImpl::AddDirtyFile(Name path)
	{
		auto it = mFileBlock.find(path);
		if (it == mFileBlock.end()) {
			return;
		}
		auto& res = it->second;
		res.MetaDirty(true);
		mDirtyBlock.push_back(&res);
	}
	void ResourceSystemImpl::SaveDirtyFile()
	{
		for (auto block : mDirtyBlock) {
			if (block->IsMetaDirty()) {
				block->MetaDirty(false);
				MetaBundle new_meta = GetVisitMeta(block->bundle);
				SaveMeta(block->path, new_meta);
			}
			if (block->IsDirty()) {
				block->Dirty(false);
				auto loader = GetLoader(PackagePath{ block->path }.GetExtension());
				loader->SaveFile(block->path, block->bundle);
			}
		}
		mDirtyBlock.clear();
	}
	void ResourceSystemImpl::SaveFile(PackagePath path, const ResourceBundle& bundle)
	{
		auto loader = GetLoader(path.GetExtension());
		loader->SaveFile(path, bundle);
	}
	void ResourceSystemImpl::LoadFileFlags()
	{
		FileHandle handle(CFileFlagName);
		handle.Open(FILE_OP::READ);
		if (handle) {
			pmr::string text = handle.ReadAll<pmr::string>();
			auto res = TextDeserialize<table<Name, uint32_t>>(text);
			if (res) {
				mFileFlags = *res;
			}
		}
	}
	void ResourceSystemImpl::SaveFileFlags()
	{
		FileHandle handle(CFileFlagName);
		handle.Open(FILE_OP::WRITE, mFileFlag & FileFlag::File_Binary);
		if (mFileFlag & FileFlag::File_Binary) {

		}
		else {
			handle.Write(TextSerialize(mFileFlags));
		}
	}
	void ResourceSystemImpl::LoadResourceFile()
	{
	}
	void ResourceSystemImpl::SaveResourceFile()
	{
		
	}
	inline uint32_t ResourceSystemImpl::GetFileFlag(Name name)
	{
		auto it = mFileFlags.find(name);
		if (it == mFileFlags.end()) {
			return 0;
		}
		return it->second;
	}
	inline void ResourceSystemImpl::SetFileFlag(Name name, uint32_t flag)
	{
		auto it = mFileFlags.find(name);
		if (it == mFileFlags.end()) {
			mFileFlags.emplace(name, flag);
		}
		else {
			it->second = flag;
		}
	}
	inline ResourceSystem::ResourceFileBlock& ResourceSystemImpl::GetFileBlock(PackagePath path) {
		return mFileBlock[Name(path())];
	}
	inline FileBlock* ResourceSystemImpl::GetResourceFile(const Guid& guid)
	{
		auto it = mResourceFile.find(guid);
		if (it == mResourceFile.end())
			return nullptr;
		return it->second;
	}
	inline ResourceSystem::ResourceSystem()
	{
		SINGLETON_PTR();
		impl = new(GlobalPool()) ResourceSystemImpl(this);
	}
	inline void api::ResourceSystem::Initialize()
	{
		impl->Initialize();
	}
	inline void ResourceSystem::Finalize()
	{
		constexpr static auto release_tables = detail::ResourceHelper::ReleaseTableResources();
		for (auto& elem : release_tables)
			elem(this);
		impl->Finalize();
	}
	inline IFileLoader* ResourceSystem::GetLoader(Name extension)
	{
		return impl->GetLoader(extension);
	}
	inline void ResourceSystem::RegisterLoader(Name ext, IFileLoader* loader)
	{
		auto ptr = impl->mFileLoader[ext];
		if (ptr) {
			delete ptr;
		}
		impl->mFileLoader[ext] = loader;
	}
	inline ResourceFileBlock& ResourceSystem::GetFileBlock(PackagePath path)
	{
		return impl->GetFileBlock(path);
	}
	inline FileBlock* ResourceSystem::GetResourceFile(const Guid& guid)
	{
		return impl->GetResourceFile(guid);
	}
	inline ResourceBundle& ResourceSystem::Load(PackagePath path, bool reload_resource, int deep)
	{
		return impl->Load(path, reload_resource, deep);
	}
	inline MetaBundle ResourceSystem::GetMeta(PackagePath path)
	{
		return impl->GetMeta(path);
	}
	inline MetaBundle ResourceSystem::GetVisitMeta(const ResourceBundle& bundle)
	{
		return impl->GetVisitMeta(bundle);
	}
	inline void ResourceSystem::AddDirtyFile(Name path)
	{
		impl->AddDirtyFile(path);
	}
	inline void ResourceSystem::SaveMeta(PackagePath path, const MetaBundle& bundle)
	{
		impl->SaveMeta(path, bundle);
	}
	inline void ResourceSystem::SaveDirtyFile()
	{
		impl->SaveDirtyFile();
	}
	inline void ResourceSystem::SaveFile(PackagePath path, const ResourceBundle& bundle)
	{
		impl->SaveFile(path, bundle);
	}
	inline void* ResourceSystem::GetTable(size_t resourceid)
	{
		return impl->mResourceTable[resourceid].get();
	}
	inline uint32_t ResourceSystem::GetFileFlag(Name name)
	{
		return impl->GetFileFlag(name);
	}
	inline void ResourceSystem::SetFileFlag(Name name, uint32_t flag)
	{
		impl->SetFileFlag(name, flag);
	}
}

