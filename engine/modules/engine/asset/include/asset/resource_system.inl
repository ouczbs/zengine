#include "resource_system.h"
#pragma once
namespace api {
	namespace detail {
		template<typename T> struct ResourceSystem_detail;

		template<typename ... Rs>
		struct ResourceSystem_detail<std::tuple<Rs...>>
		{
			constexpr static array<shared_ptr<void>, sizeof...(Rs)> GenResourceTables()
			{
				return array<shared_ptr<void>, sizeof...(Rs)>{
					std::make_shared<ResourceSystem::ResourceStorage<Rs>>()...
				};
			}

			constexpr static array<void(*)(ResourceSystem*), sizeof...(Rs)> ReleaseTableResources()
			{
				return array<void(*)(ResourceSystem*), sizeof...(Rs)>{
					[](ResourceSystem* resource_man)
						{
							if (auto table = &resource_man->GetTable<Rs>())
								table->clear();
						}...
				};
			}
		};

		using ResourceHelper = ResourceSystem_detail<Resources>;
	}
	template<typename Res>
	inline RscHandle<Res> ResourceSystem::LoadFromMeta(const Guid& guid, const SerializedMeta& meta)
	{
		if (!guid || !meta.meta) {
			return {};
		}
		Res* res = meta.meta.CastTo<Res*>();
		auto& table = GetTable<Res>();
		auto& control_block = table[guid]; // don't care just replace
		if (control_block.resource) {
			//meta.meta.MoveTo(control_block.resource);
			res = control_block.resource;
		}
		else {
			control_block.resource = res;
			res->mHandle.guid = guid;
		}
		res->Name(meta.name);
		return RscHandle<Res>{guid, res};
	}
	template<typename Res, typename ...Args>
	inline RscHandle<Res> ResourceSystem::LoadEmplaceResource(Guid guid, Args&& ...args)
	{
		auto& table = GetTable<Res>();
		auto& control_block = table[guid]; // don't care just replace
		// attempt to put on other thread
		Res* res = new Res(std::forward<Args>(args)...);
		res->mHandle.guid = guid;
		control_block.resource = res;
		return RscHandle<Res>{guid, res};
	}
	template<typename FLoader, typename ...Args>
	inline FLoader& ResourceSystem::RegisterLoader(Name ext, Args&& ...args)
	{
		FLoader *ptr = new FLoader(std::forward<Args>(args)...);
		ptr->SetFileFlag(GetFileFlag(ext));
		RegisterLoader(ext, ptr);
		return *(FLoader*)ptr;
	}
	template<typename Res>
	inline RscHandle<Res> ResourceSystem::Load(PackagePath path, bool reload_resource)
	{
		auto& res = Load(path, reload_resource);
		return res.Get<Res>();
	}
	template<typename Res>
	inline Res* ResourceSystem::Get(const RscHandle<Res>& handle, bool sync)
	{
		auto& table = GetTable<Res>();
		auto itr = table.find(handle.guid);
		if (itr == table.end()) {
			FileBlock* file = sync ? GetResourceFile(handle.guid) : nullptr;
			if (file) {
				Load(file->path);
				return Get<Res>(handle, false);
			}
			return nullptr;
		}
		return itr->second.resource;
	}

	template<typename Res>
	inline auto& ResourceSystem::GetTable()
	{
		auto ptr = GetTable(ResourceID<Res>);
		return *reinterpret_cast<ResourceStorage<Res>*>(ptr);
	}
	
	template<typename Res>
	inline ResourceSystem::ResourceBlock<Res>* ResourceSystem::GetBlock(const RscHandle<Res>& handle)
	{
		auto& table = GetTable<Res>();
		return &table[handle.guid];
	}
	template<typename Res>
	inline void RscHandle<Res>::Init()
	{
		res = ResourceSystem::Ptr()->Get(*this);
	}
}