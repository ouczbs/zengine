#pragma once
#include "refl/pch.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"
namespace refl {
    template<> struct Meta<api::Vector2> {
        using T = api::Vector2;
        using Impl = gen::MetaImpl<T, string_hash("Meta")>;
    };
}
namespace refl {
    template<> struct Meta<api::Vector3> {
        using T = api::Vector3;
        using Impl = gen::MetaImpl<T, string_hash("Meta")>;
    };
}
namespace refl {
    template<> struct Meta<api::Vector4> {
        using T = api::Vector4;
        using Impl = gen::MetaImpl<T, string_hash("Meta")>;
    };
}
namespace gen {
    template<> struct MetaImpl<api::Vector2, string_hash("Meta")> : public refl::MetaHelp {
        using T = api::Vector2;
        consteval static int MemberCount() { return 2; };
        consteval static int StaticMemberCount() { return 0; };
        consteval static int CtorCount() { return 0; };
        consteval static auto Fields() {
            return std::make_tuple(FProperty(&T::x, "x"), FProperty(&T::y, "y"));
        }
        static auto MakeFields() {
            return std::array{
                MemberField(&T::x, "x", {}),
                MemberField(&T::y, "y", {}),
            };
        };
    };
}
namespace gen {
    template<> struct MetaImpl<api::Vector3, string_hash("Meta")> : public refl::MetaHelp {
        using T = api::Vector3;
        consteval static int MemberCount() { return 3; };
        consteval static int StaticMemberCount() { return 0; };
        consteval static int CtorCount() { return 0; };
        consteval static auto Fields() {
            return std::make_tuple(FProperty(&T::x, "x"), FProperty(&T::y, "y"), FProperty(&T::z, "z"));
        }
        static auto MakeFields() {
            return std::array{
                MemberField(&T::x, "x", {}),
                MemberField(&T::y, "y", {}),
                MemberField(&T::z, "z", {}),
            };
        };
    };
}
namespace gen {
    template<> struct MetaImpl<api::Vector4, string_hash("Meta")> : public refl::MetaHelp {
        using T = api::Vector4;
        consteval static int MemberCount() { return 4; };
        consteval static int StaticMemberCount() { return 0; };
        consteval static int CtorCount() { return 0; };
        consteval static auto Fields() {
            return std::make_tuple(FProperty(&T::x, "x"), FProperty(&T::y, "y"), FProperty(&T::z, "z"), FProperty(&T::w, "w"));
        }
        static auto MakeFields() {
            return std::array{
                MemberField(&T::x, "x", {}),
                MemberField(&T::y, "y", {}),
                MemberField(&T::z, "z", {}),
                MemberField(&T::w, "w", {}),
            };
        };
    };
}