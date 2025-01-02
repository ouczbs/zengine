#pragma once
#include "memalloc.h"
#include <bit>
#include <memory_resource>
#include <iostream>
#include <numeric>
#include <windows.h>
constexpr inline size_t meta_align_size(size_t size, size_t alignment) {
    return (size + alignment - 1) / alignment * alignment;
};
namespace pmr {
    using std::pmr::memory_resource;
    class MemoryPagePool {
    public:
        static void* do_allocate(size_t bytes, size_t alignment) {
            void* ptr = (void*)VirtualAlloc(NULL, bytes, MEM_COMMIT, PAGE_READWRITE);
            return ptr;
        }
        static void do_deallocate(void* ptr) {
            if (!ptr) {
                return;
            }
            VirtualFree(ptr, 0, MEM_RELEASE);
        }
    };
    class BlockAllocator : public memory_resource {
    private:
        friend class MemoryPoolManager;
        uint16_t* m_stack;
        uint8_t*  m_bitmap; // 位图起始地址
        char*     m_data;
        uint16_t  m_block_size;
        uint16_t  m_block_count;
        uint16_t  m_used_count;
        uint16_t  m_head;
    public:
        struct MemBlock {
            uint16_t size;
            uint16_t begin_mask;
            uint16_t end_mask;
            uint16_t padding3;
            MemBlock(size_t bytes) : size(bytes)
                , begin_mask(0x5555)
                , end_mask(0xAAAA) {}
            operator bool() {
                return begin_mask == 0x5555 && end_mask == 0xAAAA;
            }
        };
        friend void dumpMetaMemoryLeaks(BlockAllocator* metaAlloc);
        BlockAllocator(size_t MemorySize, uint16_t block_size):m_block_size(block_size){
            uint16_t count = 8 * MemorySize / (8 * block_size + 8 * sizeof(uint16_t) + 1);
            uint16_t bitmap_size = (count + 7) / 8;
            size_t   offset = meta_align_size(count * 2 + bitmap_size, MEMORY_ALIGN_N);
            if (offset + count * block_size > MemorySize) {
                count--;
                offset = meta_align_size(count * 2 + bitmap_size, MEMORY_ALIGN_N);
            }
            m_stack = (uint16_t*)MemoryPagePool::do_allocate(MemorySize, MEMORY_ALIGN_N);
            m_bitmap = (uint8_t*)m_stack + count * 2;
            m_data = (char*)m_stack + offset;
            std::memset(m_bitmap, 0, bitmap_size);
            std::iota(m_stack, m_stack + count, 0);
            m_head = 0;
            m_used_count = 0;
            m_block_count = count;
        }
        ~BlockAllocator() {
            if (m_stack) {
                MemoryPagePool::do_deallocate(m_stack);
            }
            m_stack = nullptr;
        }
        bool empty() {
            return m_used_count == 0;
        }
        bool try_allocate(size_t bytes) {
            return bytes <= m_block_size || m_block_count - m_block_size >= MAX_BLOCK_ELEM_GROUP_N;
        }
        bool invert_bitmap(uint16_t bit_pos, uint8_t flag) {
            size_t  index = bit_pos / 8;
            uint8_t invert_mask = 1 << (7 - bit_pos % 8);
            m_bitmap[index] ^= invert_mask;
            return flag ? (m_bitmap[index] & invert_mask) == invert_mask :
                (m_bitmap[index] & invert_mask) == 0;
        }
        bool invert_bitmap_group(uint16_t bit_pos, uint8_t count, uint8_t flag) {
            size_t  index = bit_pos / 8;
            uint8_t offset = bit_pos % 8;
            uint8_t left_mask = 0xff >> offset;//00011111
            offset = (16 - offset - count) % 8;
            uint8_t right_mask = 0xff << offset;//11111110
            if (offset + count <= 8) {
                uint8_t mask = left_mask & right_mask;//00011110
                uint8_t mask_result = flag ? mask : 0;
                m_bitmap[index] ^= mask;
                return (m_bitmap[index] & mask) == mask_result;
            }
            m_bitmap[index] ^= left_mask;
            m_bitmap[index + 1] ^= right_mask;
            uint8_t left_mask_result = flag ? left_mask : 0;
            uint8_t right_mask_result = flag ? right_mask : 0;
            return (m_bitmap[index] & left_mask) == left_mask_result && (m_bitmap[index + 1] & right_mask) == right_mask_result;
        }
        void* do_allocate(size_t bytes, size_t alignment)override {
            uint16_t index = m_stack[m_head];
            uint16_t count = (bytes + m_block_size - 1) / m_block_size;
            if (count == 1 && !invert_bitmap(index, 1)) {
                invert_bitmap(index, 0);
                throw std::runtime_error("Error: Attempting to allocate an already allocated block.");
            }
            if (count > 1 && !invert_bitmap_group(index, count, 1)) {
                invert_bitmap_group(index, count, 1);
                throw std::runtime_error("Error: Attempting to allocate an already allocated block group.");
            }
            m_used_count += count;
            m_head = (m_head + count) % m_block_count;
            return m_data + m_block_size * index;
        }
        void do_deallocate(void* ptr, size_t bytes, size_t alignment) override {
            uint16_t index = size_t((char*)ptr - m_data) / m_block_size;
            uint16_t count = (bytes + m_block_size - 1) / m_block_size;
            if (count == 1) {
                if (!invert_bitmap(index, 0)) {
                    invert_bitmap(index, 0);
                    throw std::runtime_error("Error: Double deallocation attempt!");
                }
                m_stack[(m_head + m_block_count - m_used_count) % m_block_count] = index;
                m_used_count--;
                return;
            }
            if (count > 1 && !invert_bitmap_group(index, count, 0)) {
                invert_bitmap_group(index, count, 0);
                throw std::runtime_error("Error: Double deallocation group attempt!");
            }
            alignment = (m_head + m_block_count - m_used_count) % m_block_count;
            if (alignment + count < m_block_count) [[likely]] {
                std::iota(m_stack + alignment, m_stack + alignment + count, index);
            }
            else [[unlikely]] {
                for (uint16_t i = 0; i < count; i++) {
                    m_stack[(alignment + i) % m_block_count] = index + i;
                }
            }
            m_used_count -= count;
        }
        bool do_is_equal(const memory_resource& other) const noexcept override { return this == &other; };
    };
    class MemoryPoolManager{
    public:
        struct MemBlock {
#ifdef API_DEBUG
            void* debug_stack[MEM_CALLSTACK_DEBUG_N];
#endif // API_DEBUG
            uint32_t  begin_mask;
            uint16_t  size;
            uint8_t   index;
            uint8_t   alloc_index;
            uint16_t  size_mask;
            uint8_t   index_mask;
            uint8_t   alloc_index_mask;
            uint32_t  end_mask;
            MemBlock(size_t bytes, uint8_t index, uint8_t alloc_index)
                : size(bytes)
                , size_mask(bytes)
                , index(index)
                , alloc_index(alloc_index)
                , begin_mask(0x55555555)
                , end_mask(0xAAAAAAAA){
            }
            operator bool() {
                return size && (uint32_t)size == (uint32_t)size_mask
                    && begin_mask == 0x55555555
                    && end_mask == 0xAAAAAAAA;
            }
        };
        struct AllocatorPtr {
            BlockAllocator* alloc;
            AllocatorPtr*  next;
            AllocatorPtr() :alloc(nullptr), next(nullptr){}
            operator bool() {
                return alloc;
            }
            BlockAllocator* operator->() {
                return alloc;
            }
            ~AllocatorPtr() {
                if (alloc) {
                    alloc->~BlockAllocator();
                }
                if (next) {
                    next->~AllocatorPtr();
                    meta_free(this);
                }
            }
        };
    private:
        AllocatorPtr mAllocators[MAX_BLOCK_ELEM_SIZE_N] = {};
    public:
        friend void dumpMemoryPoolLeaks(MemoryPoolManager* memPool);
        MemoryPoolManager(){}
        ~MemoryPoolManager() {
            for (uint32_t i = 0; i < MAX_BLOCK_ELEM_SIZE_N; i++) {
                mAllocators[i].~AllocatorPtr();
                meta_free(mAllocators[i].alloc);
            }
        }
        int find_pool_index(size_t bytes) {
            size_t block_size = std::bit_ceil(bytes);
            int index = __builtin_ctz(block_size);
            if (index < MAX_BLOCK_ELEM_SIZE_N) {
                int count = ((bytes << 3) + block_size - 1) / block_size;
                if (count >= 7) {
                    return index;
                }
                if (count >= 6) {
                    return index - 2;
                }
                return index > 7 ? index - 3 : 4;
            }
            return index;
        }
        void* do_allocate(size_t bytes, size_t alignment) {
            size_t bytes_block = bytes + sizeof(MemBlock);
            int index = find_pool_index(bytes_block);
            if (index < MAX_BLOCK_ELEM_SIZE_N) {
                uint16_t alloc_index = 0;
                AllocatorPtr& pAlloc = mAllocators[index];
                if (!pAlloc) {
                    pAlloc.alloc = (BlockAllocator*)meta_malloc(sizeof(BlockAllocator));
                    new (pAlloc.alloc) BlockAllocator(MEMORY_BLOCK_SIZE, 1 << index);
                    return do_allocate_block(pAlloc.alloc, bytes_block, index, alloc_index);
                }
                while(pAlloc) {
                    if (pAlloc->try_allocate(bytes_block)) {
                        return do_allocate_block(pAlloc.alloc, bytes_block, index, alloc_index);
                    }
                    if (!pAlloc.next) {
                        void* pMemory = meta_malloc(sizeof(AllocatorPtr) + sizeof(BlockAllocator));
                        AllocatorPtr* next = new(pMemory) AllocatorPtr();
                        next->alloc = new((char*)pMemory + sizeof(AllocatorPtr)) BlockAllocator(MEMORY_BLOCK_SIZE, 1 << index);
                        pAlloc.next = next;
                        return do_allocate_block(next->alloc, bytes_block, index, alloc_index);
                    }
                    pAlloc = *pAlloc.next;
                    alloc_index++;
                }
                throw std::bad_alloc();
            }
            return malloc(bytes);
        }
        void* do_allocate_block(BlockAllocator* alloc, size_t bytes, uint8_t index, uint8_t alloc_index) {
            MemBlock* pBlock = (MemBlock*)alloc->do_allocate(bytes, 0);
            new(pBlock)MemBlock(bytes, index, alloc_index);
#ifdef API_DEBUG
            // 捕获当前堆栈
            uint16_t n = CaptureStackBackTrace(0, MEM_CALLSTACK_DEBUG_N, pBlock->debug_stack, nullptr);
#endif // API_DEBUG
            return (char*)pBlock + sizeof(MemBlock);
        }
        void do_deallocate(void* ptr) {
            if (!ptr) {
                return;
            }
            MemBlock* pBlock = (MemBlock*)((char*)ptr - sizeof(MemBlock));
            if (!*pBlock) {
                free(ptr);
                return;
            }
            AllocatorPtr& pAlloc = mAllocators[pBlock->index];
            uint16_t alloc_index = pBlock->alloc_index + 1;
            while (alloc_index && pAlloc) {
                alloc_index--;
                if (!alloc_index) {
                    pAlloc->do_deallocate(pBlock, pBlock->size, 0);
                    return;
                }
                pAlloc = *pAlloc.next;
            }
            throw std::runtime_error("Warning: deallocate erorr!!!");
        }
    };
}