#pragma once
#include "any.h"
#include "uclass.inl"
#include <iostream>
namespace refl {
	class Container {
		const void* ptr;
		const UClass_Container_Impl* cls;
		friend class Any;
		Container(const void* ptr, const UClass_Container_Impl* cls) :ptr(ptr), cls(cls) {}
	public:
		operator Any()const{
			return Any{ ptr, cls };
		}
		Any malloc(pmr::memory_resource* alloc)const {
			void* it = alloc->allocate(cls->parent->size);
			cls->Construct(it);
			return Any{ it, cls->parent };
		}
		span<const FieldPtr> GetFields() {
			return cls->parent->GetFields(EFieldFind::FIND_ALL_MEMBER, Name(""));
		}
		void construct() {
			cls->Construct((void*)ptr);
		}
		void destruct() {
			cls->Destruct((void*)ptr);
		}
		void insert(const Any& any) {
			if (any.cls == cls->parent)
				cls->vinsert(ptr, any.ptr);
		}
		int size() {
			return cls->vsize(ptr);
		}
		auto begin() {
			Container::iterator it(cls);
			cls->vbegin(it, ptr);
			return it;
		}
		auto end() {
			Container::iterator it(cls);
			cls->vend(it, ptr);
			return it;
		}
		// Iterator class
		class iterator : public UClass_Container_Impl::iterator {
		private:
			const UClass_Container_Impl* cls;
		public:
			iterator(const UClass_Container_Impl* cls)
				: cls(cls) {}
			iterator& operator++() noexcept {
				cls->viterator_add(this);
				return *this;
			}
			iterator operator++(int) noexcept {
				iterator tmp = *this;
				cls->viterator_add(this);
				return tmp;
			}
			iterator& operator--() noexcept {
				cls->viterator_sub(this);
				return *this;
			}
			iterator operator--(int) noexcept {
				iterator tmp = *this;
				cls->viterator_sub(this);
				return tmp;
			}
			Any operator->() noexcept {
				return { val , cls->parent };
			}
			Any operator*() noexcept {
				return { val , cls->parent };
			}
			constexpr bool operator!=(const iterator& other) const noexcept {
				return  cls != other.cls || ptr != other.ptr;
			}
			constexpr bool operator==(const iterator& other) const noexcept {
				return ptr == other.ptr && cls == other.cls;
			}
		};
	};
	inline Any::operator Container() const
	{
		return Container{ ptr, (const UClass_Container_Impl*)cls };
	}
}