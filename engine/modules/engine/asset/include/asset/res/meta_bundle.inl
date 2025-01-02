#pragma once
namespace api
{
	template<typename T>
	inline const SerializedMeta* MetaBundle::FetchMeta() const
	{
		Name name = meta_name<typename T::BaseResource>();
		for (auto& elem : metadatas)
		{
			if (elem.t_hash == name)
				return &elem;
		}
		return nullptr;
	}
	template<typename T>
	inline const SerializedMeta* MetaBundle::FetchMeta(Name asset_name) const
	{
		Name name = meta_name<typename T::BaseResource>();
		for (auto& elem : metadatas)
		{
			if (elem.t_hash == name && asset_name == elem.name)
				return &elem;
		}
		return nullptr;
	}
}