#pragma once
#include "yyjson.h"
#include "archive/type.h"
namespace gen {
	template<typename T, typename = void>
	struct JsonSerde {};
	template<typename T>
	struct JsonSerde<T, std::enable_if_t<is_serde_v<T>>>{
		inline static bool Read(yyjson_val* val, const void* ptr) {
			if constexpr (std::is_same_v<T, bool>) {
				*(T*)(ptr) = (T)yyjson_get_bool(val);
			}
			else if constexpr (std::is_integral_v<T>) {
				*(T*)(ptr) = (T)yyjson_get_uint(val);
			}
			else if constexpr (std::is_enum_v<T>) {
				*(T*)(ptr) = (T)yyjson_get_uint(val);
			}
			else if constexpr (std::is_floating_point_v<T>) {
				*(T*)(ptr) = (T)yyjson_get_real(val);
			}
			else if constexpr (is_string_view_v<T> || is_string_v<T>) {
				*(T*)(ptr) = yyjson_get_str(val);
			}
			else {
				//static_assert(false, "unknown json read type");
			}
			return true;
		}
		inline static yyjson_mut_val* Write(yyjson_mut_doc* doc, const void* ptr) {
			if constexpr (std::is_same_v<T, bool>) {
				return yyjson_mut_bool(doc, *(T*)(ptr));
			}
			else if constexpr (std::is_integral_v<T>) {
				return yyjson_mut_uint(doc, *(T*)(ptr));
			}
			else if constexpr (std::is_enum_v<T>) {
				return yyjson_mut_uint(doc, (std::underlying_type_t<T>)*(T*)(ptr));
			}
			else if constexpr (std::is_floating_point_v<T>) {
				return yyjson_mut_real(doc, *(T*)(ptr));
			}
			else if constexpr (is_string_view_v<T>) {
				return yyjson_mut_str(doc, std::string_view(*(T*)ptr).data());
			}
			else if constexpr (is_string_v<T>) {
				return yyjson_mut_str(doc, std::string(*(T*)ptr).data());
			}
			else {
				//static_assert(false, "unknown json write type");
			}
		}
	};
}