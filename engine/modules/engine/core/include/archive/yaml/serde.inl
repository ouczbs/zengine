#include "serde.h"
namespace gen {
	template<typename T>
	inline bool YamlRead(const YAML::Node& node, const T& t) {
		if (!node) return false;
		return YamlSerde<T>::Read(node, &t);
	}
	template<typename T>
	inline YAML::Node YamlWrite(const T& t) {
		return YamlSerde<T>::Write(&t);
	}
	template<typename T>
	struct YamlSerde<T, std::enable_if_t<refl::is_meta_v<T>>> {
		inline static bool Read(const YAML::Node& node, const void* ptr) {
			T& v = *(T*)ptr;
			if constexpr (refl::has_parent_v<T>) {
				using P = refl::parent_t<T>;
				YamlRead<refl::parent_t<T>>(node[PARENT_KEY_NAME], *(P*)ptr);
			}
			for_each_field([&](std::string_view name, auto&& value) {
				YamlRead(node[name], value);
			}, v);
			return true;
		}
		inline static YAML::Node Write(const void* ptr) {
			T& v = *(T*)ptr;
			YAML::Node node;
			if constexpr (refl::has_parent_v<T>) {
				using P = refl::parent_t<T>;
				node[PARENT_KEY_NAME] = YamlWrite<P>(*(P*)ptr);
			}
			for_each_field([&](std::string_view name, auto&& value) {
				node[name] = YamlWrite(value);
			}, v);
			return node;
		}
	};
	template<typename T>
	struct YamlSerde<T, std::enable_if_t<refl::is_container_v<T>>> {
		inline static bool Read(const YAML::Node& node, const void* ptr) {
			using value_type_t = typename T::value_type;
			T& docker = *(T*)ptr;
			value_type_t it;//构造失败怎么办？
			for (auto node_i : node) {
				if constexpr (refl::is_map_v<T>) {
					YamlRead(node_i[MAP_KEY_NAME], it.first);
					YamlRead(node_i[MAP_VALUE_NAME], it.second);
					docker[it.first] = it.second;
				}
				else {
					YamlRead(node_i, it);
					docker.push_back(it);
				}
			}
			return true;
		}
		inline static YAML::Node Write(const void* ptr) {
			T& docker = *(T*)ptr;
			YAML::Node node;
			if constexpr (refl::is_map_v<T>) {
				for (auto& it : docker) {
					YAML::Node node_i;
					node_i[MAP_KEY_NAME] = YamlWrite(it.first);
					node_i[MAP_VALUE_NAME] = YamlWrite(it.second);
					node.push_back(node_i);
				}
			}
			else {
				for (auto& it : docker) {
					node.push_back(YamlWrite(it));
				}
			}
			return node;
		}
	};
}