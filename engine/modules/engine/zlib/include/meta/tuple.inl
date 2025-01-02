#pragma once
#include <utility>
namespace meta
{
	namespace detail
	{
		template<typename T, typename ... Args, unsigned ... Indexes>
		auto tuple_construct_helper(const std::tuple<Args...>& arg_tuple, std::index_sequence<Indexes...>)
		{
			return T{(std::get<Indexes>(arg_tuple))...};
		}
	}
	template<typename T, typename ... Args>
	constexpr auto tuple_construct(const std::tuple<Args...>& arg_tuple) noexcept
	{
		return detail::tuple_construct_helper<T>(arg_tuple, std::make_index_sequence < std::tuple_size_v<std::decay_t<decltype(arg_tuple)> >> {});
	}

	template<typename ... T1s, typename ...T2s>
	struct tuple_join<std::tuple<T1s...>, std::tuple<T2s...>>
	{
		using type = std::tuple<T1s..., T2s...>;
	};

    template<typename ... Ts, template<typename ...> typename Wrap>
    struct tuple_wrap<std::tuple<Ts...>, Wrap>
    {
        using type = std::tuple<Wrap<Ts>...>;
    };
	// 泛型接口
	template <typename Tuple, typename Func, size_t... Index>
	void for_each_type_impl(Func&& func, std::index_sequence<Index...>) noexcept {
		(func.template operator() < std::tuple_element_t<Index, Tuple> > (), ...);
	}

	template <typename Tuple, typename Func>
	void for_each_type(Func&& func) noexcept {
		for_each_type_impl<Tuple>(
			std::forward<Func>(func),
			std::make_index_sequence<std::tuple_size_v<Tuple>>{}
		);
	}
}