#include "uclass.h"
#include "name.h"
#include "type.h"
namespace refl {
	template<typename T>
	class UClass_Auto : public UClass {
	public:
		UClass_Auto() : UClass(meta_name<T>(), sizeof(T)) {
			if constexpr (std::is_pointer_v<T>) {
				using RT = std::remove_pointer_t<T>;
				flag |= CLASS_POINTER_FLAG;
				if constexpr (!std::is_same_v<RT, void>) {
					parent = meta_info<RT>();
				}
			}
			else if constexpr (is_array_v<T>) {
				using RT = is_array_t<T>;
				flag = CLASS_ARRAY_FLAG;
				if constexpr (std::is_pointer_v<RT>) {
					flag |= CLASS_POINTER_FLAG;
				}
				parent = meta_info<RT>();
			}
			else {
				if constexpr (std::is_enum_v<T>) {
					flag = CLASS_ENUM_FLAG;
					parent = meta_info<std::underlying_type_t<T>>();
				}
				else if constexpr (has_parent_v<T>) {
					flag |= CLASS_PARENT_FLAG;
					parent = meta_info<parent_t<T>>();
				}
				vtable.AddConstruct(&UClass::Construct<T>);
				vtable.AddDestruct(&UClass::Destruct<T>);
			}
		}
	};
	/*
	* 模板优化
	* 成员参数转化为const void*
	* 引用参数转化为指针
	* 返回引用转化为指针
	*/
	template<typename R, typename... Args>
	class UMethod_Auto : public UClass {
		using MethodType = R(*)(Args...);
	public:
		std::array<const UClass*, sizeof...(Args) + 1> UList{};
		UMethod_Auto() : UClass(meta_name<MethodType>(), sizeof(MethodType)) {
			flag = CLASS_TRIVIAL_FLAG;
			vtable.AddGetParams(&UMethod_Auto::GetParams);
			vtable.AddCall(&UMethod_Auto::Call);
			if constexpr (!std::is_same_v<R, void>) {
				UList[0] = meta_info<R>();
			}
			if constexpr (sizeof...(Args) > 0) {
				auto ptr = &UList[1];
				(..., (*ptr = meta_info<Args>(), ptr++));
			}
		}
		//这里顺序似乎是不确定的，但是我实际运用是对的
		//如果使用make_index_sequence,会多一次函数调用
		//为什么包裹一层迭代器，就不会出现警告了
		static void Call(const FieldPtr* field, span<Any> ArgsList) {
			assert(sizeof...(Args) < ArgsList.size());
			auto param = ArgsList.end() - 1;
			if constexpr (std::is_same_v<R, void>) {
				MethodType fptr = (MethodType)field->data.method.fptr;
				fptr((param--->CastTo<Args>())...);
			}
			else {
				MethodType fptr = (MethodType)field->data.method.fptr;
				auto& ret = ArgsList.front();
				if (ret.cls == meta_info<R>()) {
					*(R*)ret.ptr = fptr((param--->CastTo<Args>())...);
				}
				else {
					fptr((param--->CastTo<Args>())...);
				}
			}
		}
		static span<const UClass*> GetParams(const UClass* cls) {
			auto& UList = ((UMethod_Auto*)cls)->UList;
			return span<const UClass*>{UList};
		}
	};
	class UClass_Container_Impl : public UClass {
	public:
		using UClass::UClass;
		struct iterator {
			void* ptr;
			void* val;
		};
		//container
		using size_impl = size_t(*)(const void*);
		using to_iterator_impl = void(*)(iterator&, const void*);
		using insert_impl = void (*)(const void*, const void*);
		//iterator
		using iterator_impl = void (*)(iterator*);
	public:
		size_impl vsize;
		to_iterator_impl vbegin;
		to_iterator_impl vend;
		insert_impl vinsert;
		iterator_impl viterator_add;
		iterator_impl viterator_sub;
		template<typename T>
		static void insert(const void* ptr, const void* obj) {
			using value_type = typename T::value_type;
			if constexpr (is_sequence_v<T>) {
				((T*)ptr)->push_back(*(value_type*)obj);
			}
			else if constexpr (is_map_v<T>) {
				((T*)ptr)->insert(*(value_type*)obj);
			}
		}
	};
	template<is_container_v T, typename value_type>
	class UClass_Container : public UClass_Container_Impl {
	public:
		static bool construct(void* ptr, const UClass* cls, span<Any> ArgsList) {
			new(ptr)T();
			return true;
		}
		static void destruct(void* ptr) {
			((T*)ptr)->~T();
		}
		static void begin(iterator& pit, const void* ptr) {
			auto it = ((T*)ptr)->begin();
			memcpy(&pit, &it, sizeof(it));
			pit.val = &*it;
		}
		static void end(iterator& pit, const void* ptr) {
			auto it = ((T*)ptr)->end();
			memcpy(&pit, &it, sizeof(it));
			pit.val = &*it;
		}
		static void add(iterator* pit) {
			auto it = ++(*(typename T::iterator*)pit);
			memcpy(pit, &it, sizeof(it));
			pit->val = &*it;
		}
		static void sub(iterator* pit) {
			auto it = --(*(typename T::iterator*)pit);
			memcpy(pit, &it, sizeof(it));
			pit->val = &*it;
		}
		UClass_Container() : UClass_Container_Impl(meta_name<T>(), sizeof(T)) {
			parent = meta_info<value_type>();
			flag = CLASS_CONTAINER_FLAG;
			vtable.AddConstruct(&UClass_Container::construct);
			vtable.AddDestruct(&UClass_Container::destruct);
			if constexpr (is_sequence_v<T>) {
				flag |= CLASS_SEQUENCE_FLAG;
			}
			else if constexpr (is_map_v<T>)
			{
				flag |= CLASS_MAP_FLAG;
			}
			auto p_size = &T::size;
			vsize = *(size_impl*)&p_size;
			vbegin = &UClass_Container::begin;
			vend = &UClass_Container::end;
			vinsert = &UClass_Container::insert<T>;

			viterator_add = &UClass_Container::add;
			viterator_sub = &UClass_Container::sub;
		};
	};
	template<typename T, typename MetaImpl>
	class UClass_Meta : public UClass {
		using FieldsType = decltype(MetaImpl::MakeFields());
	public:
		FieldsType Fields{ MetaImpl::MakeFields() };
		UClass_Meta() : UClass(meta_name<T>(), sizeof(T)) {
			if constexpr (has_parent_v<T>) {
				flag |= CLASS_PARENT_FLAG;
				parent = meta_info<parent_t<T>>();
			}
			vtable.AddGetFields(&UClass_Meta::GetFields);
			vtable.AddConstruct(&UClass::Construct<T>);
		}
		span<const FieldPtr> GetFields(EFieldFind find, const Name& name) const {
			constexpr int length = std::tuple_size<FieldsType>::value;
			constexpr int MemberCount = MetaImpl::MemberCount();
			constexpr int AllMemberCount = MemberCount + MetaImpl::StaticMemberCount();
			constexpr int CtorCount = MetaImpl::CtorCount();
			switch (find) {
			case EFieldFind::FIND_ALL_FIELD:
				return span<const FieldPtr>(&Fields[0], length);
			case EFieldFind::FIND_ALL_MEMBER:
				return span<const FieldPtr>(&Fields[0], MemberCount);
			case EFieldFind::FIND_ALL_MEMBER_WITH_STATIC:
				return span<const FieldPtr>(&Fields[0], AllMemberCount);
			case EFieldFind::FIND_ALL_METHOD:
				return span<const FieldPtr>(&Fields[AllMemberCount + CtorCount], length - AllMemberCount - CtorCount);
			case EFieldFind::FIND_CTOR:
				return span<const FieldPtr>(&Fields[AllMemberCount], CtorCount);
			case EFieldFind::FIND_FIELD:
				for (int i = 0; i < length; i++) {
					if (name == Fields[i].name) {
						return span<const FieldPtr>(&Fields[i], 1);
					}
				}
				return {};
			case EFieldFind::FIND_MEMBER:
				for (int i = 0; i < AllMemberCount; i++) {
					if (name == Fields[i].name) {
						return span<const FieldPtr>(&Fields[i], 1);
					}
				}
				return {};
			case EFieldFind::FIND_METHOD:
				for (int i = AllMemberCount + CtorCount; i < length; i++) {
					if (name == Fields[i].name) {
						return span<const FieldPtr>(&Fields[i], 1);
					}
				}
				return {};
			case EFieldFind::FIND_METHODS:
			{
				int first = 0, count = 0;
				for (int i = AllMemberCount + CtorCount; i < length; i++) {
					if (name == Fields[i].name) {
						if (!count) {
							first = i;
						}
						count++;
					}
					else if (count) {
						return span<const FieldPtr>(&Fields[first], count);
					}
				}
				return {};
			}
			default:
				return {};
			}
		}
		static span<const FieldPtr> GetFields(const UClass* _cls, EFieldFind find, Name name) {
			auto cls = static_cast<const UClass_Meta*>(_cls);
			return cls->GetFields(find, name);
		}
	};
	template<typename T>
	struct meta_info_impl {
		static UClass* create() {
			return new(MetaGlobalPool()) UClass_Auto<T>{};
		}
	};
	template<typename R, typename... Args>
	struct meta_info_impl<R(*)(Args...)> {
		static UClass* create() {
			return new(MetaGlobalPool()) UMethod_Auto<R, Args...>{};
		}
	};
	template<is_container_v T>
	struct meta_info_impl<T> {
		static UClass* create() {
			return new(MetaGlobalPool()) UClass_Container<T, typename T::value_type>{};
		}
	};
	struct MetaClasses {
		size_t hash{ 0 };
		const UClass* cls{nullptr};
		MetaClasses* next{ nullptr };
	};
	using __tClassTable = table<Name, const UClass*>;
	UNIQUER_INLINE(__tClassTable, ClassTable, "refl::ClassTable")
	using __tMetaClassTable = table<Name, MetaClasses>;
	UNIQUER_INLINE(__tMetaClassTable, MetaClassTable, "refl::MetaClassTable")
	inline const UClass* find_info(Name name) {
		auto& ClassTable = UNIQUER_VAL(ClassTable);
		if (auto it = ClassTable.find(name); it != ClassTable.end()) {
			return it->second;
		}
		return nullptr;
	}
	inline const UClass* find_info(Name name, size_t hash) {
		auto& MetaClassTable = UNIQUER_VAL(MetaClassTable);
		if (auto it = MetaClassTable.find(name); it != MetaClassTable.end()) {
			auto meta = &it->second;
			while (meta) {
				if (meta->hash == hash) {
					return meta->cls;
				}
				meta = meta->next;
			}
		}
		return nullptr;
	}
	template<typename Type>
	inline const UClass* meta_info()
	{
		using T = real_type_t<Type>;
		auto name = meta_name<T>();
		if (auto cls = find_info(name)) {
			return cls;
		}
		UClass* cls;
		if constexpr (is_meta_v<T>){
			cls = new(MetaGlobalPool()) UClass_Meta<T, meta_t<T>>{};
		}
		else if constexpr (std::is_same_v<T, void>) {
			cls = new(MetaGlobalPool()) UClass{ name, 0 };
		}
		else {
			cls = meta_info_impl<T>::create();
		}
		auto& ClassTable = UNIQUER_VAL(ClassTable);
		ClassTable[name] = cls;
		return cls;
	}
	template<typename T, size_t hash>
	inline const UClass* meta_info()
	{
		if constexpr (have_meta_impl_v<T, hash>) {
			auto name = meta_name<T>();
			const UClass* cls = find_info(name, hash);
			if (cls) {
				return cls;
			}
			cls = new(MetaGlobalPool()) UClass_Meta<T, gen::MetaImpl<T, hash>>{};
			auto& MetaClassTable = UNIQUER_VAL(MetaClassTable);
			if (auto it = MetaClassTable.find(name); it != MetaClassTable.end()) {
				auto meta = &it->second;
				while (meta) {
					if (!meta->next) {
						meta->next = new (MetaGlobalPool()) MetaClasses{ hash, cls };
						break;
					}
					meta = meta->next;
				}
			}
			else {
				MetaClassTable.emplace(name, MetaClasses{hash, cls});
			}
			return cls;
		}
		else {
			return meta_info<T>();
		}
	}
	template<typename T, size_t index = 0>
	inline void register_meta()
	{
		if constexpr (is_metas_v<T>) {
			constexpr auto metaList = Meta<T>::MetaList();
			meta_info<T, metaList[index]> ();
			if constexpr (index + 1 < metaList.size()) {
				register_meta<T, index + 1>();
			}
		}
		meta_info<T>();
	}
	inline const UClass* find_meta(Name name, size_t hash) {
		auto it = find_info(name);
		if (it) {
			return it->GetMeta(hash);
		}
		return nullptr;
	}
}