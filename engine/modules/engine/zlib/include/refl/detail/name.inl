#include "name.h"
namespace refl {
	namespace detail {
		template<typename T>
		constexpr std::string_view func_signature() noexcept {
#  if defined(__clang__)
			auto sign = std::string_view{ __PRETTY_FUNCTION__ };
			return sign.substr(53, sign.size() - 54);
#  elif defined(__GNUC__)
			auto sign = std::string_view{ __PRETTY_FUNCTION__ };
			return sign.substr(62, sign.size() - 62);
#  elif defined(_MSC_VER)
			auto sign = std::string_view{ __FUNCSIG__ };
			return sign.substr(48, sign.size() - 48);
#  endif
		}
		template<typename T>
		constexpr auto raw_meta_tstr() noexcept {
			constexpr std::string_view sign = func_signature<T>();
			constexpr size_t size = sign.size();
			return TStr<size>{ sign };
		}
		template<size_t N, size_t M>
		constexpr auto concat(const TStr<N>& lhs, const TStr<M>& rhs) {
			TStr<N + M> result;
			std::copy(lhs.value.begin(), lhs.value.end(), result.value.begin());
			std::copy(rhs.value.begin(), rhs.value.end(), result.value.begin() + N);
			return result;
		}

		constexpr int _num_digits(int num) {
			return num < 10 ? 1 : 1 + _num_digits(num / 10);
		}
		template <int N, int Digits = _num_digits(N)>
		constexpr auto num_tstr() {
			char data[Digits + 1];
			int n = N;
			for (int i = Digits - 1; i >= 0; --i) {
				data[i] = static_cast<char>('0' + n % 10);
				n /= 10;
			}
			return TStr{ data };
		}
		template<typename T>
		constexpr auto num_prefix_tstr() {
			if constexpr (std::is_integral_v<T>) {
				if constexpr (std::is_signed_v<T>) {
					return TStr{ "S" };
				}
				else {
					return TStr{ "U" };
				}
			}
			else if constexpr (std::is_floating_point_v<T>) {
				return TStr{ "F" };
			}
			else {
				return TStr{ "D" };
			}
		}
	}
	template<auto v>
	constexpr auto value_tstr()noexcept {
		using T = decltype(v);
		if constexpr (std::is_null_pointer_v<T>)
			return TStr{ "nullptr" };
		else if constexpr (std::is_pointer_v<T>) {
			if constexpr (v == nullptr)
				return TStr{ "nullptr" };
			else
				static_assert("not support");
		}
		else if constexpr (std::is_integral_v<T>) {
			if constexpr (std::is_same_v<T, bool>) {
				if constexpr (v == true)
					return TStr{ "true" };
				else
					return TStr{ "false" };
			}
			else {
				return TStr{ "false" };
			}
		}
	}
	template<typename Args1, typename... Args>
	constexpr auto concat_seq(Args1 args1, Args&&... args) {
		if constexpr (sizeof...(Args) == 0) {
			return args1;
		}
		else if constexpr (sizeof...(Args) == 1) {
			return detail::concat(args1, args...);
		}
		else {
			return detail::concat(args1, concat_seq(args...));
		}
	}
	template<typename T>
	constexpr auto meta_tstr() noexcept {
		if constexpr (std::is_same_v<T, void>) {
			return TStr{ "void" };
		}
		else if constexpr (std::is_arithmetic_v<T>) {
			constexpr auto prefix = detail::num_prefix_tstr<T>();
			constexpr auto bit = detail::num_tstr<8 * sizeof(T)>();
			return concat_seq(prefix, bit);
		}
		else if constexpr (std::is_array_v<T>) {
			constexpr auto r = std::rank_v<T>;
			constexpr auto ex = std::extent_v<T, 0>;
			if constexpr (r == 1) {
				if constexpr (ex == 0)
					return concat_seq(TStr{"[]{"}, meta_tstr<std::remove_extent_t<T>>(), TStr{"}"});
				else
					return concat_seq(TStr{"["}, detail::num_tstr<ex>(), TStr{"]{"}, meta_tstr<std::remove_extent_t<T>>(), TStr{"}"});
			}
			else { // r > 1
				static_assert(r > 1);
				if constexpr (ex == 0)
					return concat_seq(TStr{"[]"}, type_name<std::remove_extent_t<T>>());
				else
					return concat_seq(TStr{"["}, detail::num_tstr<ex>(), TStr{"]"}, meta_tstr<std::remove_extent_t<T>>());
			}
		}
		else {
			return detail::raw_meta_tstr<T>();
		}
	}
	template<typename T>
	Name meta_name() noexcept
	{
		constexpr auto tstr = meta_tstr<T>();
		return Name(tstr.view());
	}
}