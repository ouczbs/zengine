#pragma once
#include <type_traits>
namespace meta {
    template<typename Enum>
    concept is_enum_t = std::is_enum_v<Enum>;
}
template<typename Enum>
concept is_enum_t = std::is_enum_v<Enum>;
template<meta::is_enum_t Enum>
inline constexpr Enum& operator|=(Enum& lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    lhs = static_cast<Enum>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
    return lhs;
}
template<meta::is_enum_t Enum>
inline constexpr Enum operator|(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) | underlying(rhs));
}
template<meta::is_enum_t Enum>
inline constexpr Enum operator&(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) & underlying(rhs));
}
template<meta::is_enum_t Enum>
inline constexpr bool any(Enum lhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return (underlying)lhs != 0;
}