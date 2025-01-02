#pragma once
#include "guid.h"
#include "resource_config.h"

namespace api
{
	struct RscHandleBase {
		UPROPERTY({})
		Guid guid;
		void* res{nullptr};
	};
	template<typename Res>
	struct RscHandle : public RscHandleBase
	{
		constexpr size_t RscID()const { return ResourceID<Res>; }
		constexpr RscHandle() noexcept = default;
		template<typename T>
		constexpr RscHandle(const RscHandle<T>& other) noexcept : RscHandleBase(other.guid, other.res) {};
		constexpr RscHandle(const Guid& guid, Res* res) noexcept : RscHandleBase(guid, res) {}
		void Init();
		void Clear() { res = nullptr; };
		Res* Ptr() {
			return (Res*)res;
		}
		Res* operator->() { if (!res && guid) Init();  return (Res*)res; }
		Res& operator*() { if (!res && guid) Init(); return *(Res*)res; }
		operator bool() { if (!res && guid) Init(); return res; }
		Res* operator->() const { return (Res*)res; }
		Res& operator*()const { return *(Res*)res; }
		operator bool() const { return res; }
	};
}
#include "resource_handle.inl"
#include ".asset/resource_handle_gen.inl"