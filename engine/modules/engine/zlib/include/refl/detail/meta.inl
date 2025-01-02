#include "meta.h"
#include "uclass.h"
namespace refl {
	inline span<Any> MakeAnyArgs(span<Any> args, span<const UClass*> params, pmr::memory_resource* alloc = MetaGlobalPool()) {
		size_t clsIndex = params.size() - args.size();
		assert(clsIndex > 0);
		span<const UClass*> subParams = params.subspan(clsIndex);
		clsIndex = subParams.size();
		int size = meta_align_size(clsIndex * sizeof(Any));
		for (auto cls : subParams) {
			size += meta_align_size(cls->size);//数据大小
		}
		void* memory = (char*)alloc->allocate(size);
		Any* any = (Any*)memory;
		char* pData = (char*)memory + clsIndex * sizeof(Any);
		for (size_t i = 0; i < clsIndex; i++) {
			any->cls = subParams[i];
			any->ptr = pData;
			assert(Convert::Construct(*any, args[i]));
			any++;
			pData += meta_align_size(any->cls->size);
		}
		return span<Any>{(Any*)memory, clsIndex};
	}
	template<typename T, typename ...Args>
	static void NewCtor(void* mem, Args... args) {
		new (mem) T(std::forward<Args>(args)...);
	}
	template<typename T, typename ...Args>
	inline FieldPtr MetaHelp::CtorField(T(*ptr)(Args...), const MethodData& data)
	{
		const UClass* cls = meta_info<void(*)(const void*, real_type_t<Args>...)>();
		uint32_t flag = FIELD_CTOR_FLAG;
		MethodData method;
		if (!data.value.empty()) {
			flag |= FIELD_METHOD_VALUE_FLAG;
			method.value = MakeAnyArgs(data.value, cls->GetParams());
		}
		if (data.meta) {
			method.meta = data.meta.New(MetaGlobalPool());
		}
		method.fptr = (Method)&NewCtor<T, Args...>;
		return FieldPtr{Name("Ctor"), cls, method, flag};
	}
	template<typename T, typename Obj>
	inline FieldPtr MetaHelp::MemberField(T Obj::* ptr, std::string_view name, const MemberData& data)
	{
		const UClass* cls = meta_info<T>();
		uint32_t flag = FIELD_MEMBER_FLAG;
		MemberData member;
		member.offset = reinterpret_cast<std::size_t>(&(reinterpret_cast<const Obj*>(0)->*ptr));
		if (data.value) {
			member.value = data.value.New(MetaGlobalPool());
		}
		if (data.meta) {
			member.meta = data.meta.New(MetaGlobalPool());
		}
		return FieldPtr{ name, cls, member, flag };
	}
	template<typename T, typename Obj>
	inline FieldPtr MetaHelp::MemberField(T* ptr, std::string_view name, const MemberData& data)
	{
		const UClass* cls = meta_info<T>();
		uint32_t flag = FIELD_STATIC_MEMBER_FLAG;
		MemberData member;
		member.offset = reinterpret_cast<std::size_t>(&(reinterpret_cast<const Obj*>(0)->*ptr));
		if (data.value) {
			member.value = data.value.New(MetaGlobalPool());
		}
		if (data.meta) {
			member.meta = data.meta.New(MetaGlobalPool());
		}
		return FieldPtr{ name, cls, member, flag };
	}
	template<typename R, typename ...Args>
	inline FieldPtr MetaHelp::MethodField(R(*ptr)(Args...), std::string_view name, const MethodData& data)
	{
		MethodData method;
		uint32_t flag = FIELD_METHOD_FLAG;
		const UClass* cls = meta_info<real_type_t<R>(*)(real_type_t<Args>...)>();
		if (!data.value.empty()) {
			flag |= FIELD_METHOD_VALUE_FLAG;
			method.value = MakeAnyArgs(data.value, cls->GetParams());
		}
		if (data.meta) {
			method.meta = data.meta.New(MetaGlobalPool());
		}
		method.fptr = { *(Method*)&ptr };
		return { name, cls, method,flag };
	}
	template<typename T, typename R, typename ...Args>
	inline FieldPtr MetaHelp::MethodField(R(T::* ptr)(Args...), std::string_view name, const MethodData& data) {
		MethodData method;
		uint32_t flag = FIELD_METHOD_FLAG;
		const UClass* cls = meta_info<real_type_t<R>(*)(const void*, real_type_t<Args>...)>();
		if (!data.value.empty()) {
			flag |= FIELD_METHOD_VALUE_FLAG;
			method.value = MakeAnyArgs(data.value, cls->GetParams());
		}
		if (data.meta) {
			method.meta = data.meta.New(MetaGlobalPool());
		}
		method.fptr = { *(Method*)&ptr };
		return { name, cls, method,flag };
	}
}