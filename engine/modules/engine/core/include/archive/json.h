#pragma once
#include "json/serialize.inl"
#include "json/serde.inl"
#ifdef API_DEBUG
#define JSON_WRITE_FLAGS YYJSON_WRITE_PRETTY
#else
#define JSON_WRITE_FLAGS YYJSON_WRITE_NOFLAG
#endif
namespace gen {
	template<is_any_v T>
	struct JsonSerde<T> {
		inline static bool Read(yyjson_val* val, const void* ptr) {
			return api::JsonArchive::Deserialize(val, refl::Any{ ptr, refl::meta_info<refl::Any>() });
		}
		inline static yyjson_mut_val* Write(yyjson_mut_doc* doc, const void* ptr) {
			return api::JsonArchive::Serialize(doc, refl::Any{ ptr, refl::meta_info<refl::Any>() });
		}
	};
}
namespace api {
	namespace detail {
		// 辅助结构体，用于检查特化版本
		template<typename T, typename = void>
		struct has_json_specialization : std::false_type {};
		// 特化辅助结构体，匹配特化版本
		template <typename T>
		struct has_json_specialization<T, std::void_t<decltype(gen::JsonSerde<T>::Read)>> : std::true_type {};
	}
	template<typename T>
	concept has_json_specialization_v = detail::has_json_specialization<T>::value;
	template<typename T>
	inline bool JsonDeserialize(string_view text, T* obj) {
		if (text.empty()) return false;
		yyjson_alc alc = JsonAllocatorAdapter();
		yyjson_doc* doc = yyjson_read_opts((char*)text.data(), text.size(), YYJSON_READ_INSITU, &alc, nullptr);
		if (!doc) return false;
		yyjson_val* root = yyjson_doc_get_root(doc);
		if constexpr (has_json_specialization_v<T>) {
			return gen::JsonSerde<T>::Read(root, obj);
		}
		else {
			return JsonArchive::Deserialize(root, obj);
		}
	}
	template<typename T, typename... Args>
	inline result<T, SerializeError> JsonDeserialize(string_view text, Args&& ...args) {
		if (text.empty()) {
			return SerializeError::SERDE_EMPTY;
		}
		T* obj = new(FramePool())T(std::forward<Args>(args)...);
		bool bsuccess = JsonDeserialize<T>(text, obj);
		using ResultType = result<T, SerializeError>;
		return bsuccess ? ResultType{ *obj } : ResultType{ SerializeError::SERDE_ERROR };
	}
	template<has_json_specialization_v T>
	inline string_view JsonSerialize(const T& t) {
		yyjson_alc alc = JsonAllocatorAdapter();
		yyjson_mut_doc* doc = yyjson_mut_doc_new(&alc);
		yyjson_mut_val* root = gen::JsonSerde<T>::Write(doc, &t);
		yyjson_mut_doc_set_root(doc, root);
		size_t len;
		const char* json_str = yyjson_mut_write_opts(doc, JSON_WRITE_FLAGS, &alc, &len, NULL);
		return json_str ? string_view(json_str, len) : string_view("");
	}
	inline string_view JsonSerialize(Any any) {
		yyjson_alc alc = JsonAllocatorAdapter();
		yyjson_mut_doc* doc = yyjson_mut_doc_new(&alc);
		yyjson_mut_val* root = JsonArchive::Serialize(doc, any);
		yyjson_mut_doc_set_root(doc, root);
		size_t len;
		const char* json_str = yyjson_mut_write_opts(doc, JSON_WRITE_FLAGS, &alc, &len, NULL);
		return json_str ? string_view(json_str, len) : string_view("");
	}
}