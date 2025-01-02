#pragma once
#if defined(__cplusplus)
#include <new>
#include "xmalloc_type.h"
void operator delete(void* p) noexcept { xfree(p); };
void operator delete[](void* p) noexcept { xfree(p); };

void operator delete  (void* p, const std::nothrow_t&) noexcept { xfree(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { xfree(p); }

void* operator new(std::size_t n) noexcept(false) { return xmalloc(n); }
void* operator new[](std::size_t n) noexcept(false) { return xmalloc(n); }

void* operator new  (std::size_t n, const std::nothrow_t& tag) noexcept { (void)(tag); return xmalloc_nothrow(n); }
void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept { (void)(tag); return xmalloc_nothrow(n); }

#if (__cplusplus >= 201402L || _MSC_VER >= 1916)
void operator delete  (void* p, std::size_t n) noexcept { xfree_size(p, n); };
void operator delete[](void* p, std::size_t n) noexcept { xfree_size(p, n); };
#endif

#if (__cplusplus > 201402L || defined(__cpp_aligned_new))
void operator delete  (void* p, std::align_val_t al) noexcept { xfree_aligned(p, static_cast<size_t>(al)); }
void operator delete[](void* p, std::align_val_t al) noexcept { xfree_aligned(p, static_cast<size_t>(al)); }
void operator delete  (void* p, std::size_t n, std::align_val_t al) noexcept { xfree_size_aligned(p, n, static_cast<size_t>(al)); };
void operator delete[](void* p, std::size_t n, std::align_val_t al) noexcept { xfree_size_aligned(p, n, static_cast<size_t>(al)); };
void operator delete  (void* p, std::align_val_t al, const std::nothrow_t&) noexcept { xfree_aligned(p, static_cast<size_t>(al)); }
void operator delete[](void* p, std::align_val_t al, const std::nothrow_t&) noexcept { xfree_aligned(p, static_cast<size_t>(al)); }

void* operator new  (std::size_t n, std::align_val_t al) noexcept(false) { return xmalloc_aligned(n, static_cast<size_t>(al)); }
void* operator new[](std::size_t n, std::align_val_t al) noexcept(false) { return xmalloc_aligned(n, static_cast<size_t>(al)); }
void* operator new  (std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept { return xmalloc_aligned_nothrow(n, static_cast<size_t>(al)); }
void* operator new[](std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept { return xmalloc_aligned_nothrow(n, static_cast<size_t>(al)); }
#endif
#endif
