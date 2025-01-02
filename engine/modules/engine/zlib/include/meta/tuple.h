#pragma once
// tuple utils

#include <tuple>

namespace meta
{
	template<typename Tuple, template<typename ...> typename Wrap>
	struct tuple_wrap;

	template<typename Tuple, template<typename ...> typename Wrap>
	using tuple_wrap_t = typename tuple_wrap<Tuple, Wrap>::type;


	template<typename...Ts>
	using tuple_cat_t = decltype(std::tuple_cat(std::declval<Ts>()...));

	template<typename T1, typename T2>
	struct tuple_join;

	template<typename T1, typename T2>
	using tuple_join_t = typename tuple_join<T1, T2>::type;

	template <typename T, typename Tuple, std::size_t Index = 0>
	consteval std::size_t index_in_tuple() {
		if constexpr (Index == std::tuple_size_v<Tuple>) {
			return Index; // 如果没有找到，返回元组的大小作为标志
		}
		else if constexpr (std::is_same_v<T, std::tuple_element_t<Index, Tuple>>) {
			return Index; // 找到匹配的类型，返回当前索引
		}
		else {
			return index_in_tuple<T, Tuple, Index + 1>(); // 递归检查下一个索引
		}
	}

	template<typename FindMe, typename Tuple>
	constexpr uint8_t index_in_tuple_v = index_in_tuple<FindMe, Tuple>();

	template<typename T, typename ... Args>
	constexpr auto tuple_construct(const std::tuple<Args...>&) noexcept;
	template <typename Tuple, typename Func>
	void for_each_type(Func&& func)noexcept;
}
#include "tuple.inl"