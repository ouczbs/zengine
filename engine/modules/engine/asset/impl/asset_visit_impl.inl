#include "asset/asset.h"
#include "render/asset/mesh.h"
#include "render/asset/texture.h"
#include <algorithm>
namespace api {
	template <class T>
	concept is_meta_v = requires(T t) { { t.Meta() } -> std::same_as<refl::Any>;};
	void FindIncludes(MetaBundle& bundle, refl::Any meta) {
		auto fieldList = meta.cls->GetFields(refl::FIND_ALL_MEMBER, Name(""));
		auto cls = meta_info<RscHandleBase>();
		for (auto field : fieldList) {
			if (field.type == cls) {
				RscHandleBase* base = meta.Member(field).CastTo<RscHandleBase*>();
				FileBlock* file = ResourceSystem::Ptr()->GetResourceFile(base->guid);
				if (file) {
					bundle.includes.push_back(file->path);
				}
			}
		}
	}
	MetaBundle ResourceSystemImpl::GetVisitMeta(const ResourceBundle& bundle)
	{
		MetaBundle new_meta = MetaBundle{};
		for (auto& elem : bundle.GetAll())
			std::visit([&](auto& handle) {
				using T = std::decay_t<decltype(*handle)>;
				SerializedMeta meta{ handle.guid, handle->Name(), meta_name<T>() };
				if constexpr (is_meta_v<T>) {
					meta.meta = handle->Meta();
				}
				new_meta.Add(meta);
				FindIncludes(new_meta, meta.meta);
			}, elem);
		auto& v = new_meta.includes;
		if (v.size() > 1) {
			std::sort(v.begin(), v.end());  // 首先排序
			auto end_unique = std::unique(v.begin(), v.end());  // 去除重复元素
			v.erase(end_unique, v.end());
		}
		return new_meta;
	}
	void AssetModule::InitResourceType()
	{
		meta::for_each_type<Resources>([]<typename T>() {
			refl::register_meta<T>();
		});
	}
}