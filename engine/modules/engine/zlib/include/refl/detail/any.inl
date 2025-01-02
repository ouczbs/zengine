#include "any.h"
#include "uclass.h"
#include "convert.h"
namespace refl{
	inline bool Any::HasParent() const
	{
		return cls->flag & CLASS_PARENT_FLAG;
	}
	inline bool Any::IsEnum() const
	{
		return cls->flag & CLASS_ENUM_FLAG;
	}
	inline bool Any::IsArray() const
	{
		return cls->flag & CLASS_ARRAY_FLAG;
	}
	inline bool Any::IsObject() const
	{
		return !(cls->flag & CLASS_ARRAY_FLAG) && !(cls->flag & CLASS_POINTER_FLAG);
	}
	inline bool Any::IsContainer() const
	{
		return cls->flag & CLASS_CONTAINER_FLAG;
	}
	inline bool Any::IsSequence() const
	{
		return cls->flag & CLASS_SEQUENCE_FLAG;
	}
	inline bool Any::IsMap() const
	{
		return cls->flag & CLASS_MAP_FLAG;
	}
	inline const UClass* Any::ContainerType() const
	{
		if (IsContainer()) {
			return cls->parent;
		}
		return nullptr;
	}
	inline int Any::ArraySize() const
	{
		if (cls->flag & CLASS_ARRAY_FLAG) {
			return cls->size / cls->parent->size;
		}
		return 0;
	}
	inline void Any::CopyTo(void* ptr) const
	{
		Any to{ ptr, cls };
		auto fieldList = cls->GetFields(refl::FIND_ALL_MEMBER, Name(""));
		for (auto& field : fieldList) {
			Any obj = to.Member(field);
			Convert::Construct(obj, Member(field));
		}
	}
    inline bool Any::Check(const UClass* parent)const
    {
		if (cls == parent) {
			return true;
		}
		if (!cls || !parent) {
			return false;
		}
		return cls->IsChildOf(parent);
    }
	inline bool Any::Construct(span<Any> ArgsList) const
	{
		return cls->Construct((void*)ptr, ArgsList);
	}
	inline void Any::Destruct() const
	{
		cls->Destruct((void*)ptr);
	}
	inline Any Any::New(pmr::memory_resource* pool) const
	{
		void* data = pool->allocate(cls->size);
		Any any{ data, cls };
		Convert::Construct(any, *this);
		return any;
	}
	template<typename T>
	inline T Any::FindVtable(Name name) const
	{
		return (T)cls->vtable.Find(name);
	}
	inline Any Any::Member(const FieldPtr& field)const
	{
		if (field.flag & FIELD_MEMBER_FLAG) {
			return { (const char*)ptr + field.data.member.offset, field.type };
		}
		return {};
	}
	inline Any Any::Member(int i) const
	{
		if (cls->flag & CLASS_ARRAY_FLAG) {
			int offset = i * cls->parent->size;
			if (offset < cls->size)
				return { (const char*)ptr + offset, cls->parent };
		}
		return Any();
	}

	inline Any Any::Parent() const
	{
		if(cls->parent)
			return Any{ptr, cls->parent};
		return {};
	}

	inline bool UClass::Construct(void* ptr, span<Any> ArgsList) const
	{
		auto construct = vtable.Construct();
		if (construct) {
			if (construct(ptr, this, ArgsList)) {
				return true;
			}
			auto fieldList = GetFields(EFieldFind::FIND_CTOR, Name("Ctor"));
			if (fieldList.empty()) {
				memset(ptr, 0, size);
				return false;
			}
			std::array<Any, MAX_ARGS_LENGTH> ArgsArray = { Any{} , Any{ptr} };
			int i = 2;
			for (auto& arg : ArgsList) {
				ArgsArray[i++] = arg;
			}
			span<Any> FieldArgs(&ArgsArray[0], 2 + ArgsList.size());
			if (fieldList.size() == 1) {
				return fieldList[0].Invoke<true>(FieldArgs);
			}
			//todo: 函数重载需要更精确的处理吗？
			for (auto& field : fieldList) {
				if (field.Invoke<true>(FieldArgs)) {
					return true;
				}
			}
			memset(ptr, 0, size);
			return false;
		}
		if (!ArgsList.empty() && ArgsList[0].Check(this)) {
			memcpy(ptr, ArgsList[0].ptr, size);
			return true;
		}
		memset(ptr, 0, size);
		return true;
	}
	inline Any FieldPtr::Construct(void* ptr) const
	{
		Any obj{ ptr, type };
		if (data.member.value && flag & FIELD_MEMBER_FLAG) {
			obj.Construct(data.member.value);
		}
		else {
			obj.Construct({});
		}
		return obj;
	}
}