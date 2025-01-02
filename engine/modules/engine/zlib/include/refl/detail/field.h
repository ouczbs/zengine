#pragma once
#include "any.h"
namespace refl {
	using Offset = uint32_t;
	using Method = void*;
	struct MemberData {
		Offset offset{ 0 };
		Any value;
		Any meta;
		constexpr MemberData() :value(), meta() {}
		constexpr MemberData(const Any& value, const Any& meta = {}) : value(value), meta(meta) {}
		constexpr MemberData(Offset offset, const Any& value, const Any& meta) : offset(offset), value(value), meta(meta) {}
	};
	struct MethodData {
		Method fptr{ nullptr };
		span<Any> value;
		Any meta;
		constexpr MethodData() :value(), meta() {}
		constexpr MethodData(span<Any> value, const Any& meta = {}) : value(value), meta(meta) {}
		constexpr MethodData(Method fptr, span<Any> value, const Any& meta) : fptr(fptr), value(value), meta(meta) {}
	};
	enum FieldFlag :uint32_t {
		FIELD_NONE_FLAG = 0,
		FIELD_MEMBER_FLAG = 1 << 0,
		FIELD_STATIC_MEMBER_FLAG = 1 << 1,
		FIELD_METHOD_FLAG = 1 << 2,
		FIELD_METHOD_VALUE_FLAG = 1 << 3,
		FIELD_CTOR_FLAG = 1 << 4,
	};
	struct FieldPtr {
		union Data
		{
			MemberData member;
			MethodData method;
			constexpr Data() : member() {};
			constexpr Data(const MemberData& member) :member(member) {}
			constexpr Data(const MethodData& method) : method(method) {}
			constexpr Data(Offset offset) : member(offset, {}, {}) {}
			constexpr Data(Offset offset, const Any& value, const Any& meta) : member(offset, value, meta) {}
			constexpr Data(Method fptr, span<Any> value, const Any& meta) : method(fptr, value, meta) {}
		};
		Name name;
		const UClass* type{};
		Data data{};
		uint32_t flag{};
		bool IsStatic()const {
			return flag & FIELD_STATIC_MEMBER_FLAG;
		}
		Offset GetOffset() const {
			if (flag & FIELD_MEMBER_FLAG) {
				return data.member.offset;
			}
			return 0;
		}
		Any Construct(void* ptr)const;
		Any GetValue()const {
			if (flag & FIELD_MEMBER_FLAG)
				return data.member.value;
			return {};
		}
		Any GetMeta()const {
			if (flag & FIELD_MEMBER_FLAG)
				return data.member.meta;
			if (flag & FIELD_METHOD_FLAG)
				return data.method.meta;
			return {};
		}
		template<typename Func, typename... Args>
		auto Call(Func func, Args&&... args)const;
		template<bool IsSafeMemory = false>
		bool Invoke(span<Any> ArgsList)const;
	};
}