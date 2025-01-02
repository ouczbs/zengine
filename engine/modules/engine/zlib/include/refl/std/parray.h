#include "refl/pch.h"
namespace refl {
	template<typename T>
	class parray {
	protected:
		const UClass* m_cls;
		T* m_ptr;
		int m_count;
	public:
		parray(): m_cls(nullptr), m_ptr(nullptr),m_count(0) {}
		template<typename C>
		requires std::is_base_of_v<T, C>
		parray(std::vector<C>& vec) : m_cls(meta_info<C>()), m_ptr(nullptr){
			m_count = vec.size();
			if (m_count > 0) {
				C* ptr = new C[m_count];
				m_ptr = ptr;
				if constexpr (std::is_trivially_copyable_v<C>) {
					memcpy(m_ptr, vec.data(), m_count * sizeof(C));
				}
				else {
					for (int i = 0; i < m_count; i++) {
						new(ptr + i) C(vec[i]);
					}
				}
			}
		}
		parray(parray<T>& parr) : m_cls(parr.m_cls)
			, m_ptr(parr.data()), m_count(parr.size()) {
			parr.reset();
		}
		~parray() {
			if (!m_count || !m_cls)
				return;
			auto dest = m_cls->vtable.Destruct();
			if (dest) {
				for (int i = 0; i < m_count; i++) {
					dest((char*)m_ptr + (i * m_cls->size));
				}
			}
			m_count = 0;
			m_ptr = nullptr;
		}
		T* at(int i)noexcept {
			if(i < m_count)
				return (T*)((char*)m_ptr + (i * m_cls->size));
			return nullptr;
		}
		T* data() noexcept{
			return m_ptr;
		}
		void reset() noexcept {
			m_ptr = nullptr;
			m_count = 0;
		}
		int size() const noexcept {
			return m_count;
		}
		int data_size()const noexcept {
			return m_count * m_cls->size;
		}
		int capicty() const noexcept {
			return m_count * m_cls->size;
		}
		const UClass* uclass()const noexcept {
			return m_cls;
		}
	};
	template<typename T, typename C>
	parray<T> ToParray(std::vector<C>& vec){
		return parray<T>{vec};
	}
}
