#pragma once
#include "pmr/name.h"
#include <array>
#include <string>
#include <cassert>
#include <span>
namespace refl {
	using std::span;
	using pmr::Name;
	using pmr::table;
	namespace detail {
		// 定义一个模板结构体用于检测是否为 数组
		template<typename T>
		struct is_array : std::false_type {};

		template<typename T, size_t N>
		struct is_array<T[N]> : std::true_type {
			using type = T;
			consteval static size_t size() { return N; }
		};
		// 部分特化用于匹配 std::array
		template<typename T, size_t N>
		struct is_array<std::array<T, N>> : std::true_type {
			using type = T;
			consteval static size_t size() { return N; }
		};

		// 定义一个模板结构体用于检测是否为 std::pair
		template<typename T>
		struct is_pair : std::false_type {};

		template<typename T1, typename T2>
		struct is_pair<std::pair<T1, T2>> : std::true_type {
			using key_type = T1;
    		using value_type = T2;
		};

		// 定义一个模板结构体用于检测是否为 std::pair
		template<typename T>
		struct is_tuple : std::false_type {};

		template<typename... Types>
		struct is_tuple<std::tuple<Types...>> : std::true_type {};
	}
	template<typename T>
	concept is_array_v = detail::is_array<T>::value;
	template<typename T>
	using	is_array_t = detail::is_array<T>::type;

	template<typename T>
	concept is_pair_v = detail::is_pair<T>::value;

	template<typename T>
	concept is_tuple_v = detail::is_tuple<T>::value;

	template<typename T>
	concept is_string_v = requires(T t) {
		{ static_cast<std::string>(t) } -> std::convertible_to<std::string>;
	};

	template<typename T>
	concept is_container_v = !is_array_v<T> && !is_string_v<T> && requires(T a) {
		{ a.begin() } -> std::input_iterator;
		{ a.end() } -> std::input_iterator;
	};

	template<typename T>
	concept is_map_v = is_pair_v<typename T::value_type> && is_container_v<T>;

	template<typename T>
	concept is_sequence_v = !is_pair_v<typename T::value_type> && is_container_v<T>;
};
namespace refl {
	namespace detail {
		template<typename T>
		struct real_type {
			using type = std::remove_cv_t<T>;
		};
		template<typename T>
		struct real_type<T&> {
			using type = std::remove_cv_t<T>*;
		};
		template<typename T>
		struct real_type<T*> {
			using type = std::remove_cv_t<T>*;
		};
		//转化为指针类型
		template<typename T>
		struct args_type {
			using type = std::remove_cv_t<T>;
		};
		template<typename T>
		struct args_type<T&> {
			using type = std::remove_cv_t<T>;
		};
		template<typename T>
		struct args_type<T*> {
			using type = std::remove_cv_t<T>;
		};
	}
	template<typename T>
	using real_type_t = detail::real_type<T>::type;
	template<typename T>
	using args_type_t = detail::args_type<T>::type;
};
namespace gen {
	template<typename T, size_t hash>
	struct MetaImpl;
}
namespace refl {
	template<class T>
	struct Meta {};

	template<class T>
	concept is_meta_v = requires { typename Meta<T>::Impl; };

	template <class T>
	concept is_metas_v = requires() { Meta<T>::MetaList(); };

	template <class T, size_t hash>
	concept have_meta_impl_v = requires() { typename gen::MetaImpl<T, hash>::T; };

	template <class T>
	concept has_parent_v = requires { typename Meta<T>::Parent; };

	template<class T>
	using meta_t = Meta<T>::Impl;
	template<class T>
	using parent_t = typename Meta<T>::Parent;

	//类型接口
	struct UClass;
	template<typename Type>
	const UClass* meta_info();
	const UClass* find_info(Name name, size_t hash);
	constexpr uint32_t MAX_ARGS_LENGTH = 10;
}