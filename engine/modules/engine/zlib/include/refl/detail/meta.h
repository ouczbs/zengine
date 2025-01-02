#pragma once
#include "field.h"
namespace gen {
	using refl::real_type_t;
	template<typename T, size_t hash>
	struct MetaImpl {};
	template<typename T>
	struct Property {
		std::string_view name;
		T ptr;
		constexpr Property(T ptr, std::string_view name) noexcept:name(name), ptr(ptr) {}
	};
	template<typename T>
	consteval Property<T> FProperty(T ptr, std::string_view name) { return { ptr, name }; }

	// 遍历所有字段值
	template <typename Func, typename T, size_t hash = string_hash("Meta")>
	inline void for_each_field(Func&& func, T& obj) {
		constexpr auto props = MetaImpl<T, hash>::Fields();
		std::apply([&](auto... field) {
			((func(field.name, obj.*(field.ptr))), ...);
			}, props);
	}
}
namespace refl {
	struct MetaHelp{
		template<typename T, typename R, typename ...Args>
		static constexpr auto remove_const(R(T::* ptr)(Args...) const) {
			using MethodType = R(T::*)(Args...);
			return (MethodType)ptr;
		}
		template<typename T, typename... Args>
		static FieldPtr CtorField(T(*ptr)(Args...), const MethodData& data = {});

		template<typename T, typename Obj>
		static FieldPtr MemberField(T Obj::* ptr, std::string_view name, const MemberData& data = {});

		template<typename T, typename Obj>
		static FieldPtr MemberField(T* ptr, std::string_view name, const MemberData& data = {});

		template<typename R, typename ...Args>
		static FieldPtr MethodField(R(*ptr)(Args...), std::string_view name, const MethodData& data = {});

		template<typename T, typename R, typename ...Args>
		static FieldPtr MethodField(R(T::* ptr)(Args...), std::string_view name, const MethodData& data = {});

		template<typename T, typename R, typename ...Args>
		static FieldPtr MethodField(R(T::* ptr)(Args...)const, std::string_view name, const MethodData& data = {}) {
			return MethodField(remove_const(ptr), name, data);
		}
	};
}