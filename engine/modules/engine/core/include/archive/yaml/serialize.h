#pragma once
#include "serde.h"
namespace api {
	using refl::Any;
	using refl::UClass;
	using refl::meta_info;
	struct YamlVTable {
		using Read = bool (*)(const YAML::Node&, const void*);
		using Write = YAML::Node(*)(const void*);
	};
	struct YamlArchive {
	public:
		static void InitSerde();
	public:
		template<typename T>
		static void Register();
		static YAML::Node Serialize(Any any);
		static bool Deserialize(const YAML::Node& node, Any any);
	};
}