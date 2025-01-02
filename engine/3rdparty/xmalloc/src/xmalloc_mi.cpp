#include "xmalloc_type.h"
#include "mimalloc.h"
void clean() {
	// 强制垃圾回收并检查未释放的内存
	mi_stats_print(NULL);
	mi_collect(true);
}
struct mi_exit_wrap {
	mi_exit_wrap() {
		std::atexit(clean);
	}
};
static mi_exit_wrap _mi_exit_wrap;
XMALLOC_API void* xmalloc(size_t size){return mi_malloc(size);}
XMALLOC_API void* xmalloc_nothrow(size_t size) noexcept{return mi_new_nothrow(size);}

XMALLOC_API void xfree(void* p) noexcept{mi_free(p);}

#if (__cplusplus >= 201402L || _MSC_VER >= 1916)
XMALLOC_API void xfree_size(void* p, size_t size) noexcept{return mi_free_size(p, size);}
#endif

#if (__cplusplus > 201402L || defined(__cpp_aligned_new))
XMALLOC_API void xfree_size_aligned(void* p, size_t size, size_t alignment) noexcept{return mi_free_size_aligned(p, size, alignment);}
XMALLOC_API void xfree_aligned(void* p, size_t alignment) noexcept{return mi_free_aligned(p, alignment);}

XMALLOC_API void* xmalloc_aligned(size_t size, size_t alignment) { return mi_new_aligned(size, alignment); }
XMALLOC_API void* xmalloc_aligned_nothrow(size_t size, size_t alignment) noexcept { return mi_new_aligned_nothrow(size, alignment); }
#endif