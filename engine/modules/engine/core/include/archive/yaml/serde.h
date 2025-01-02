#pragma once
#include "yaml-cpp/yaml.h"
#include "archive/type.h"
namespace gen {
	template<typename T, typename = void>
	struct YamlSerde {};
	template<typename T>
	struct YamlSerde<T, std::enable_if_t<is_serde_v<T>>> {
		inline static bool Read(const YAML::Node& node, const void* ptr) {
			if constexpr (std::is_same_v<T, bool>) {
				*(T*)(ptr) = node.as<bool>();
			}
			else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
				*(T*)(ptr) = node.as<T>();
			}
			else if constexpr (std::is_enum_v<T>) {
				*(T*)(ptr) = (T)node.as<uint32_t>();
			}
			else if constexpr (is_string_view_v<T> || is_string_v<T>) {
				*(T*)(ptr) = node.Scalar();
			}
			else {
				//static_assert(false, "unknown json read type");
			}
			return true;
		}
		inline static YAML::Node Write(const void* ptr) {
			if constexpr (std::is_same_v<T, bool> || std::is_integral_v<T> || std::is_floating_point_v<T>) {
				return YAML::Node{*(T*)ptr};
			}
			else if constexpr (std::is_enum_v<T>) {
				return YAML::Node{ *(uint32_t*)(T*)ptr };
			}
			else if constexpr (is_string_view_v<T>) {
				return YAML::Node{ std::string_view(*(T*)ptr).data() };
			}
			else if constexpr (is_string_v<T>) {
				return YAML::Node{ std::string(*(T*)ptr).data() };
			}
			else {
				//static_assert(false, "unknown json write type");
			}
		}
	};
}