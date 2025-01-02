#pragma once
#include "resource_system.h"
namespace api {
	class Asset : public Resource<Asset> {
	public:
		using Base = Resource<Asset>;
		const refl::UClass* meta;
		Asset(const refl::UClass* meta): Base(), meta(meta){}
		refl::Any Meta() {
			return refl::Any{ this, meta };
		}
	};
}