#pragma once
XMALLOC_API void* xmalloc(size_t size);
XMALLOC_API void* xmalloc_nothrow(size_t size) noexcept;
XMALLOC_API void  xfree(void* p) noexcept;
#if (__cplusplus >= 201402L || _MSC_VER >= 1916)
XMALLOC_API void  xfree_size(void* p, size_t size)                           noexcept;
#endif
#if (__cplusplus > 201402L || defined(__cpp_aligned_new))
XMALLOC_API void  xfree_size_aligned(void* p, size_t size, size_t alignment) noexcept;
XMALLOC_API void  xfree_aligned(void* p, size_t alignment)                   noexcept;
XMALLOC_API void* xmalloc_aligned(size_t size, size_t alignment);
XMALLOC_API void* xmalloc_aligned_nothrow(size_t size, size_t alignment) noexcept;
#endif