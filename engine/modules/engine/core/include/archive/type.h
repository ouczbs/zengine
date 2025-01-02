#pragma once
#include "meta/result.h"
#include "refl/pch.h"
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