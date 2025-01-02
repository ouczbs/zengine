#pragma once
#include "pmr/name.h"
#include <array>
#include <string_view>
namespace refl {
    using pmr::Name;
    template <size_t N>
    struct TStr {
        std::array<char, N> value{}; // 确切长度的字符数组，不依赖 '\0'
        constexpr TStr() {};
        constexpr TStr(const char(&data)[N + 1]) {
            std::copy_n(data, N, value.begin());
        }

        constexpr TStr(std::string_view data) {
            std::copy_n(data.begin(), std::min(N, data.size()), value.begin());
        }

        // 直接返回 std::string_view 类型，无需额外处理末尾字符
        constexpr std::string_view view() const {
            return std::string_view(value.data(), N);
        }
    };

    template <size_t N>
    TStr(const char(&)[N]) -> TStr<N - 1>;

    template<typename T>
    Name meta_name()noexcept;
}