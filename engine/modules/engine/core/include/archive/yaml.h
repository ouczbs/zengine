#include "yaml/serde.inl"
#include "yaml/serialize.inl"
namespace gen {
	template<is_any_v T>
	struct YamlSerde<T> {
		inline static bool Read(const YAML::Node& node, const void* ptr) {
			return api::YamlArchive::Deserialize(node, refl::Any{ ptr, refl::meta_info<refl::Any>() });
		}
		inline static YAML::Node Write(const void* ptr) {
			return api::YamlArchive::Serialize(refl::Any{ ptr, refl::meta_info<refl::Any>() });
		}
	};
}
namespace YAML {
	inline Node APILoad(std::string_view text) {
		std::stringstream stream{ std::string(text) };
		return Load(stream);
	}
	inline std::string_view APIDump(const Node& root) {
		Emitter emitter;
		emitter << root;
		const char* data = emitter.c_str();
		size_t size = emitter.size();
		// 在 FramePool 上分配内存并拷贝数据
		char* buffer = static_cast<char*>(FramePool()->allocate(size + 1, alignof(std::max_align_t)));
		std::memcpy(buffer, data, size);
		buffer[size] = 0x00;
		// 返回指向缓冲区的 string_view
		return std::string_view(buffer, size);
	}
}
namespace api {
	namespace detail {
		// 辅助结构体，用于检查特化版本
		template<typename T, typename = void>
		struct has_yaml_specialization : std::false_type {};
		// 特化辅助结构体，匹配特化版本
		template <typename T>
		struct has_yaml_specialization<T, std::void_t<decltype(gen::YamlSerde<T>::Read)>> : std::true_type {};
	}
	template<typename T>
	concept has_yaml_specialization_v = detail::has_yaml_specialization<T>::value;
	template<typename T>
	inline bool YamlDeserialize(string_view text, T* obj) {
		if (text.empty()) return false;
		YAML::Node root = YAML::APILoad(text);
		if constexpr (has_yaml_specialization_v<T>) {
			return gen::YamlSerde<T>::Read(root, obj);
		}
		else {
			return YamlArchive::Deserialize(root, obj);
		}
	}
	template<typename T, typename... Args>
	inline result<T, SerializeError> YamlDeserialize(string_view text, Args&& ...args) {
		if (text.empty()) {
			return SerializeError::SERDE_EMPTY;
		}
		T* obj = new(FramePool())T(std::forward<Args>(args)...);
		bool bsuccess = YamlDeserialize<T>(text, obj);
		using ResultType = result<T, SerializeError>;
		return bsuccess ? ResultType{ *obj } : ResultType{ SerializeError::SERDE_ERROR };
	}
	template<has_yaml_specialization_v T>
	inline string_view YamlSerialize(const T& t) {
		YAML::Node root = gen::YamlSerde<T>::Write(&t);
		return YAML::APIDump(root);
	}
	inline string_view YamlSerialize(Any any) {
		YAML::Node root = YamlArchive::Serialize(any);
		return YAML::APIDump(root);
	}
}