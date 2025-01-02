#pragma once
#include <condition_variable>
#include <semaphore>
#include <concepts>
namespace zstd {
	template<typename T>
	class channel {
	protected:
		int m_tail;
		int m_head;
		int m_size;
		T* m_buf;
		std::mutex m_mtx;
		std::condition_variable m_cv;
	public:
		~channel() {
			reset();
			free(m_buf);
		}
		channel(int size = 1) : m_tail(0), m_head(0), m_buf((T*)malloc(sizeof(T)* size)), m_size(size) {}
		int head() {
			return m_head;
		}
		int tail() {
			return m_tail;
		}
		int count() {
			return m_head - m_tail;
		}
		void reset() {
			for (int i = m_tail; i < m_head; i++) {
				std::destroy_at(m_buf + (m_tail % m_size));
			}
			m_tail = 0;
			m_head = 0;
		}
		template<typename... Args>
		void release(Args... args) {
			std::unique_lock<std::mutex> lck(m_mtx);
			while (m_head >= m_tail + m_size) {
				m_cv.wait(lck);
			}
			std::construct_at(m_buf + m_head % m_size, std::forward<Args>(args)...);
			if(m_head++ == m_tail)
				m_cv.notify_one();
		};
		T acquire() {//不能传右值引用，右值引用拷贝地址数据不在锁保护范围内
			std::unique_lock<std::mutex> lck(m_mtx);
			while (m_tail >= m_head) {
				m_cv.wait(lck);
			}
			int tail = m_tail % m_size;
			if(m_tail++ == m_head - m_size)
				m_cv.notify_one();
			return std::move(*(m_buf + tail));
		};
	};
}