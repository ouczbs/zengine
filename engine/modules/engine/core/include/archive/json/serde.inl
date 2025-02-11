#include "serde.h"
namespace gen {
	template<typename T>
	inline bool JsonRead(yyjson_val* node, const T& t) {
		if (!node) return false;
		return JsonSerde<T>::Read(node, &t);
	}
	template<typename T>
	inline yyjson_mut_val* JsonWrite(yyjson_mut_doc* doc, const T& t) {
		return JsonSerde<T>::Write(doc, &t);
	}
	template<typename T>
	struct JsonSerde<T, std::enable_if_t<refl::is_meta_v<T>>> {
		inline static bool Read(yyjson_val* val, const void* ptr) {
			T& v = *(T*)ptr;
			for_each_field([=](std::string_view name, auto&& value) {
				JsonRead(yyjson_obj_get(val, name.data()), value);
			}, v);
			return true;
		}
		inline static yyjson_mut_val* Write(yyjson_mut_doc* doc, const void* ptr) {
			T& v = *(T*)ptr;
			yyjson_mut_val* obj = yyjson_mut_obj(doc);
			for_each_field([=](std::string_view name, auto&& value) {
				yyjson_mut_obj_add_val(doc, obj, name.data(), JsonWrite(doc, value));
			}, v);
			return obj;
		}
	};
	template<typename T>
	struct JsonSerde<T, std::enable_if_t<refl::is_container_v<T>>> {
		inline static bool Read(yyjson_val* val, const void* ptr) {
			using value_type_t = typename T::value_type;
			T& docker = *(T*)ptr;
			size_t length = yyjson_arr_size(val);
			value_type_t it;//构造失败怎么办？
			for (size_t i = 0; i < length; ++i) {
				yyjson_val* obj_i = yyjson_arr_get(val, i);
				if constexpr (refl::is_map_v<T>) {
					JsonRead(yyjson_obj_get(obj_i, MAP_KEY_NAME), it.first);
					JsonRead(yyjson_obj_get(obj_i, MAP_VALUE_NAME), it.second);
					docker[it.first] = it.second;
				}
				else {
					JsonRead(obj_i, it);
					docker.push_back(it);
				}
			}
			return true;
		}
		inline static yyjson_mut_val* Write(yyjson_mut_doc* doc, const void* ptr) {
			T& docker = *(T*)ptr;
			yyjson_mut_val* arr = yyjson_mut_arr(doc);
			if constexpr (refl::is_map_v<T>) {
				for (auto& it : docker) {
					yyjson_mut_val* obj = yyjson_mut_obj(doc);
					yyjson_mut_obj_add_val(doc, obj, MAP_KEY_NAME, JsonWrite(doc, it.first));
					yyjson_mut_obj_add_val(doc, obj, MAP_VALUE_NAME, JsonWrite(doc, it.second));
					yyjson_mut_arr_add_val(arr, obj);
				}
			}
			else {
				for (auto& it : docker) {
					yyjson_mut_arr_add_val(arr, JsonWrite(doc, it));
				}
			}
			return arr;
		}
	};
}