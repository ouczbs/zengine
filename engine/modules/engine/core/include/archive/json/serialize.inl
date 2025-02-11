#include "serialize.h"
namespace api {
	// 定义 yyjson_alc 适配器类
	inline yyjson_alc JsonAllocatorAdapter(std::pmr::memory_resource* mr = FramePool()) {
		// 初始化 yyjson_alc 结构体
		yyjson_alc alc;
		alc.malloc = [](void* ctx, size_t size) -> void* {
			auto* adapter = static_cast<std::pmr::memory_resource*>(ctx);
			return adapter->allocate(size);
			};
		alc.realloc = [](void* ctx, void* ptr, size_t old_size, size_t size) -> void* {
			auto* adapter = static_cast<std::pmr::memory_resource*>(ctx);
			// std::pmr::memory_resource 不支持直接重新分配
			// 因此，先分配新的内存，再拷贝数据
			void* new_ptr = adapter->allocate(size);
			if (ptr) {
				std::memcpy(new_ptr, ptr, std::min(old_size, size));
				adapter->deallocate(ptr, old_size);
			}
			return new_ptr;
			};
		alc.free = [](void* ctx, void* ptr) {
			auto* adapter = static_cast<std::pmr::memory_resource*>(ctx);
			adapter->deallocate(ptr, 0);
			};
		alc.ctx = mr;
		return alc;
	}
	inline void JsonArchive::InitSerde()
	{
		using std::string_view;
#define RegisterAny(T) Register<T>();
#include "../register.inl"
#undef RegisterAny
	}
	inline Name VJsonSerdeRead() {
		return Name("JsonSerdeRead");
	}
	inline Name VJsonSerdeWrite() {
		return Name("JsonSerdeWrite");
	}
	template<typename T>
	inline void JsonArchive::Register()
	{
		auto uclass = meta_info<T>();
		auto [bfind, it] = uclass->vtable.FindLast(VJsonSerdeRead());
		if (!bfind && it) {
			it = it->Insert(VJsonSerdeRead(),  (void*) &gen::JsonSerde<T>::Read);
			it = it->Insert(VJsonSerdeWrite(), (void*) &gen::JsonSerde<T>::Write);
		}
	}
	inline yyjson_mut_val* JsonArchive::Serialize(yyjson_mut_doc* doc, Any any)
	{
		if (!any) {
			return {};
		}
		auto it = any.FindVtable<JsonVTable::Write>(VJsonSerdeWrite());
		if (it) {
			return it(doc, any.ptr);
		}
		if (any.IsContainer()) {
			return {};
		}
		if (any.IsArray()) {
			return {};
		}
		yyjson_mut_val* obj = yyjson_mut_obj(doc);
		if (auto p = any.Parent()) {
			yyjson_mut_obj_add_val(doc, obj, PARENT_KEY_NAME, Serialize(doc, p));
		}
		auto fieldList = any.cls->GetFields(refl::FIND_ALL_MEMBER, pmr::Name(""));
		for (auto& field : fieldList) {
			yyjson_mut_obj_add_val(doc, obj, field.name.data(), JsonArchive::Serialize(doc, any.Member(field)));
		}
		return obj;
	}
	inline bool JsonArchive::Deserialize(yyjson_val* res, Any any)
	{
		if (!any) {
			return false;
		}
		auto it = any.FindVtable<JsonVTable::Read>(VJsonSerdeRead());
		if (it) {
			return it(res, any.ptr);
		}
		if (any.IsContainer()) {
			bool isMap = any.IsMap();
			refl::Container docker = any;
			auto fieldList = docker.GetFields();
			for (auto obj : docker) {
				if (isMap) {
					Any first = obj.Member(fieldList[0]);
					Any second = obj.Member(fieldList[1]);
					//result[Serialize(first)] = Serialize(second);
				}
				else {
					//result.push_back(Serialize(obj));
				}
			}
		}
		if (any.IsArray()) {
			return false;
		}
		return false;
	}
}