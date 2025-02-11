#pragma once
#include "meta/result.h"
#include "refl/pch.h"
#define PARENT_KEY_NAME	"__parent__"
#define CLASS_KEY_NAME	"__class__"
#define DATA_KEY_NAME	"__data__"
#define MAP_KEY_NAME	"#k"
#define MAP_VALUE_NAME	"#v"

namespace gen {
	template<typename T>
	struct Meta {};
	template<typename T>
	concept is_string_v = requires(T t) {
		{ static_cast<std::string>(t) } -> std::same_as<std::string>;
	};
	template<typename T>
	concept is_string_view_v = requires(T t) {
		{ static_cast<std::string_view>(t) } -> std::same_as<std::string_view>;
	};
	template<typename T>
	concept is_serde_v = std::is_same_v<T, bool> || std::is_integral_v<T> || std::is_floating_point_v<T> || is_string_v<T> || std::is_enum_v<T>
		|| is_string_view_v<T>;

	template<typename T>
	concept is_any_v = std::is_base_of_v<refl::Any, T>;
}
namespace api {
	using meta::result;
	using std::string_view;
	using pmr::Name;
	enum class SerializeError : char
	{
		SERDE_EMPTY,
		SERDE_ERROR,
	};
}