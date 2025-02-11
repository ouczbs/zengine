#pragma once
#include "type.h"
namespace refl {
	class UClass;
	class Container;
	class FieldPtr;
	struct Any;
	template<typename T>
	concept is_not_any_v = !std::is_same_v<args_type_t<T>, Any>;
	struct Any {
	public:
		const void* ptr;
		const UClass* cls;
	public:
		constexpr Any() noexcept: ptr(nullptr), cls(nullptr) {}
		constexpr Any(const void* ptr, const UClass* cls) noexcept : ptr(ptr), cls(cls) {}
		template<is_not_any_v T>
		constexpr Any(T&& v) noexcept : ptr(&v), cls(meta_info<std::remove_reference_t<T>>()) {}
		template<is_not_any_v T>
		constexpr Any(T* v) noexcept : ptr(v), cls(meta_info<T>()) {}
		template<typename T>//参数 T* => T*
		constexpr inline T CastTo() const {
			if constexpr (std::is_pointer_v<T>) {
				return (T)ptr;
			}
			else if constexpr (std::is_reference_v<T>) {
				using RT = std::remove_reference_t<T>;
				return *(RT*)ptr;
			}
			else {
				return *(T*)ptr;
			}
		}
	public:
		operator span<Any>() const {
			return span<Any>{(Any*)this, 1};
		}
		operator Container()const;
		operator bool()const { return cls && ptr; }
		bool Check(const UClass* parent) const;
		template<typename T>
		T FindVtable(Name name)const;
		bool Construct(span<Any> ArgsList) const;
		void Destruct()const;
		Any New(pmr::memory_resource* pool)const;

		Any Member(const FieldPtr& field)const;
		Any Member(int i)const;
		Any Parent()const;
		
		bool HasParent()const;
		bool IsEnum()const;
		bool IsArray()const;
		bool IsObject()const;
		bool IsContainer()const;
		bool IsSequence()const;
		bool IsMap()const;
		const UClass* ContainerType()const;
		int ArraySize()const;
		void CopyTo(void* ptr)const;
		void MoveTo(void* ptr)const {
			CopyTo(ptr);
			Destruct();
		}
	};
	template<typename T>
	class TAny : public Any{
	public:
		TAny() : Any(){}
		TAny(T* t) : Any(t){}
		T* operator->() {
			return (T*)ptr;
		}
	};
	template<typename T> struct Meta<TAny<T>> {
		using Parent = Any;
	};
}