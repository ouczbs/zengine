#pragma once
#include <vector>
#include <memory_resource>
namespace pmr {
    using std::pmr::memory_resource;
    using std::pmr::vector;
    class ZLIB_API FrameAllocator : public memory_resource {
    private:
        char* buffer;
        size_t capacity;
        size_t offset;
    public:
        // 删除拷贝构造函数
        FrameAllocator(const FrameAllocator&) = delete;
        FrameAllocator& operator=(const FrameAllocator&) = delete;
        FrameAllocator(FrameAllocator&& o) noexcept;
        FrameAllocator& operator=(FrameAllocator&& o)noexcept;
    public:
        FrameAllocator(size_t size) noexcept;
        ~FrameAllocator() noexcept;
        bool empty() const { return offset == 0; }
        void reset() { offset = 0; };
        friend void* try_allocate(FrameAllocator& allocator, size_t bytes, size_t alignment);
    private:
        void* do_allocate(size_t bytes, size_t alignment)override;
        void move_clear() { buffer = nullptr; capacity = 0; offset = 0; };
        void do_deallocate(void* p, size_t bytes, size_t alignment) override {};
        bool do_is_equal(const memory_resource& other) const noexcept override { return this == &other; };
    };
    class FrameAllocatorPool : public memory_resource {
    public:
        FrameAllocatorPool(size_t allocatorSize = 1024 * 1024) noexcept : allocatorSize(allocatorSize) {}

        void* allocate(size_t bytes, size_t alignment);
        void reset();
        void* do_allocate(size_t bytes, size_t alignment) override { return allocate(bytes, alignment); }
        void do_deallocate(void* p, size_t bytes, size_t alignment) override {};
        bool do_is_equal(const memory_resource& other) const noexcept override { return this == &other; };
    private:
        size_t allocatorSize;
        vector<FrameAllocator> allocators{};
    };
};
//自定义的new操作符 内存只分配，不单独释放，不能调用delete
inline void* operator new(size_t size, pmr::FrameAllocatorPool* pool, size_t alignment = alignof(std::max_align_t)) {
    size = (size + alignment - 1) & ~(alignment - 1);
    return pool->allocate(size, alignment);
}
//全局生命周期，不回收内存
//局部生命周期，每帧回收内存
ZLIB_API extern pmr::FrameAllocatorPool* MetaGlobalPool();
ZLIB_API extern pmr::FrameAllocatorPool* GlobalPool();
ZLIB_API extern pmr::FrameAllocatorPool* FramePool();
#include "frame_allocator.inl"