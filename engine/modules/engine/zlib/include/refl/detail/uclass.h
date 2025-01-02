#pragma once
#include "field.h"
namespace refl {
	template<typename T>
	using pmr_vector = std::pmr::vector<T>;
	enum ClassFlag :uint32_t {
		CLASS_NONE_FLAG = 0,
		CLASS_TRIVIAL_FLAG = 1 << 0,
		CLASS_POINTER_FLAG = 1 << 1,
		CLASS_ARRAY_FLAG = 1 << 2,
		CLASS_CONTAINER_FLAG = 1 << 3,
		CLASS_SEQUENCE_FLAG = 1 << 4,
		CLASS_MAP_FLAG = 1 << 5,
		CLASS_PARENT_FLAG = 1 << 6,
		CLASS_ENUM_FLAG = 1 << 7,
	};
	enum EFieldFind :uint32_t {
		FIND_ALL_FIELD = 0,
		FIND_ALL_MEMBER,
		FIND_ALL_MEMBER_WITH_STATIC,
		FIND_ALL_METHOD,
		FIND_FIELD,
		FIND_CTOR,
		FIND_MEMBER,
		FIND_METHOD,
		FIND_METHODS,//函数重载 特别是构造函数
	};
	struct vtable_uclass
	{   
		struct Node {
			Name name;
			void* method;
			Node* next;
			Node(Name name, void* ptr) :name(name), method(ptr), next(nullptr) {}
			Node* Insert(Name name, void* ptr) {
				Node* it = new Node(name, ptr);
				next = it;
				return it;
			}
		};
		Node* header{nullptr};
		operator bool() { return header; }
		void Add(Name name, void* ptr) {
			auto [bfind, it] = FindLast(name);
			if (bfind) {
				return;
			}
			Node* next = new Node(name, ptr);
			Node** prev= it ? &(it->next) : &header;
			*prev = next;
		}
		std::pair<bool,Node*> FindLast(Name name) const{
			Node *prev = nullptr, *it = header;
			while (it) {
				if (it->name == name) {
					return std::make_pair(true, it);
				}
				prev = it;
				it = it->next;
			}
			return std::make_pair(false, prev);
		}
		void* Find(Name name) const {
			Node* it = header;
			while (it) {
				if (it->name == name) {
					return it->method;
				}
				it = it->next;
			}
			return nullptr;
		}
		//class
		using GetFields_t = span<const FieldPtr>(*)(const UClass*, EFieldFind find, Name name);
		//function
		using GetParams_t = span<const UClass*>(*)(const UClass*);
		//function args
		using Call_t = void (*)(const FieldPtr*,span<Any> ArgsList);
		//object
		using Construct_t = bool (*)(void* ptr, const UClass* cls,span<Any> ArgsList);
		using Destruct_t = void (*)(void*);
		void AddGetFields(GetFields_t func) {
			Add(Name("GetFields"), (void*)func);
		}
		void AddGetParams(GetParams_t func) {
			Add(Name("GetParams"), (void*)func);
		}
		void AddCall(Call_t func) {
			Add(Name("Call"), (void*)func);
		}
		void AddConstruct(Construct_t func) {
			Add(Name("Construct"), (void*)func);
		}
		void AddDestruct(Destruct_t func) {
			Add(Name("Destruct"), (void*)func);
		}
		GetFields_t GetFields() const { return (GetFields_t)	Find(Name("GetFields")); }
		GetParams_t GetParams() const { return (GetParams_t)	Find(Name("GetParams")); }
		Call_t		Call()		const { return (Call_t)			Find(Name("Call")); }
		Construct_t Construct() const { return (Construct_t)	Find(Name("Construct")); }
		Destruct_t	Destruct()	const { return (Destruct_t)		Find(Name("Destruct")); }
	};
	class UClass {
	public:
		Name name;
		uint32_t size : 16;
		uint32_t flag : 16{0};
		const UClass* parent;
		vtable_uclass vtable;
		UClass(const UClass*) = delete;
		UClass& operator=(const UClass*) = delete;
	public:
		UClass(Name name, uint32_t size, const UClass* parent = nullptr)
			:name(name), size(size), parent(parent) {}
		Any New(pmr::memory_resource* pool, span<Any> ArgsList = {}) const {
			void* ptr = pool->allocate(size);
			Construct(ptr, ArgsList);
			return { ptr , this };
		}
		bool Construct(void* ptr, span<Any> ArgsList = {})const;
		void Destruct(void* ptr)const {
			auto func = vtable.Destruct();
			if (func) {
				func(ptr);
			}
		}
		span<const FieldPtr> GetFields(EFieldFind find, const Name& name)const {
			auto func = vtable.GetFields();
			if (func) {
				return func(this, find, name);
			}
			return {};
		}
		span<const UClass*> GetParams()const {
			auto func = vtable.GetParams();
			if (func) {
				return func(this);
			}
			return {};
		}
		const UClass* GetMeta(size_t hash)const {
			return find_info(name, hash);
		}
		bool IsChildOf(const UClass* cls, bool bthis = false) const {
			const UClass* _parent = bthis ? this : parent;
			while (_parent != nullptr) {
				if (_parent == cls) {
					return true;
				}
				_parent = _parent->parent;
			}
			return false;
		}
		template<typename T>
		bool IsChildOf(bool bthis = false) const {
			return IsChildOf(meta_info<T>(), bthis);
		}
	public:
		template<typename T>
		static void Destruct(void* ptr) {
			std::destroy_at((T*)ptr);
		}
		template<typename T>
		static bool Construct(void* ptr, const UClass* cls, span<Any> ArgsList = {}) {
			int argsSize = ArgsList.size();
			if (argsSize == 0) {
				if constexpr (std::is_constructible_v<T>) {
					std::construct_at((T*)ptr);
					return true;
				}
				return false;
			}
			if (argsSize == 1 && ArgsList[0].Check(cls)) {
				*(T*)ptr = *(const T*)ArgsList[0].ptr;
				return true;
			}
			return false;
		}
	};
}