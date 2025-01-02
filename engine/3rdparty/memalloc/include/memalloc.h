#pragma once
#define MEMORY_BLOCK_SIZE 1048576
#define META_BLOCK_EMEM_SIZE 16
#define MAX_BLOCK_ELEM_SIZE_N 16
#define MAX_BLOCK_ELEM_GROUP_N 8
#define MEM_CALLSTACK_DEBUG_N 16
#define MEMORY_ALIGN_N 16
namespace pmr {
    void* meta_malloc(size_t bytes, size_t alignment = MEMORY_ALIGN_N);
    void  meta_free(void* ptr);
}

MEMALLOC_API void* me_malloc(size_t size);
MEMALLOC_API void* me_malloc_nothrow(size_t size) noexcept;
MEMALLOC_API void  me_free(void* p) noexcept;
#if (__cplusplus >= 201402L || _MSC_VER >= 1916)
MEMALLOC_API void  me_free_size(void* p, size_t size)                           noexcept;
#endif
#if (__cplusplus > 201402L || defined(__cpp_aligned_new))
MEMALLOC_API void* me_malloc_aligned(size_t size, size_t alignment);
MEMALLOC_API void* me_malloc_aligned_nothrow(size_t size, size_t alignment) noexcept;
MEMALLOC_API void  me_free_size_aligned(void* p, size_t size, size_t alignment) noexcept;
MEMALLOC_API void  me_free_aligned(void* p, size_t alignment)                   noexcept;
#endif