#pragma once
#include "type.h"
#include "guid.h"
#include "resource_bundle.h"
namespace api
{
	struct SerializedMeta
	{
		UPROPERTY({})
		Guid   guid;
		UPROPERTY({})
		Name name;
		UPROPERTY({})
		Name t_hash;
		UPROPERTY({})
		refl::Any meta;
		bool operator==(const SerializedMeta& other)const{
			return guid == other.guid && name == other.name && t_hash == other.t_hash;
		}
	};
	struct ResourceBundle;
	struct MetaBundle
	{
		UPROPERTY({})
		vector<SerializedMeta> metadatas;
		UPROPERTY({})
		vector<Name> includes;
		MetaBundle() = default;

		const SerializedMeta* FetchMeta(const Guid& guid) const;
		template<typename T>
		const SerializedMeta* FetchMeta() const;
		template<typename T>
		const SerializedMeta* FetchMeta(Name asset_name) const;
		void Add(const SerializedMeta& meta) {
			metadatas.push_back(meta);
		};
		bool operator==(const MetaBundle& other)const;
		bool operator!=(const MetaBundle& other)const;
	};
}
#include "meta_bundle.inl"
#include ".asset/meta_bundle_gen.inl"