#pragma once
#include "serde.h"
namespace api {
	using refl::Any;
	using refl::UClass;
	using refl::meta_info;
	struct JsonVTable {
		using Read = bool(*)(yyjson_val*, const void*);
		using Write = yyjson_mut_val*(*)(yyjson_mut_doc*, const void*);
	};
	struct JsonArchive {
	public:
		static void InitSerde();
	public:
		template<typename T>
		static void Register();
		static yyjson_mut_val* Serialize(yyjson_mut_doc* doc, Any any);
		static bool Deserialize(yyjson_val* res, Any any);
	};
}