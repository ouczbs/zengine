//内存设计有问题，溜了溜了。还是先学习mimalloc吧
#include "memory_pool_debug.h"
namespace pmr {
	inline MemoryPoolManager* pMainPool;
	inline BlockAllocator* pMetaAlloc;
	void CleanMainMemPool() {
		if (!pMainPool) return;
		dumpMemoryPoolLeaks(pMainPool);
		pMainPool->~MemoryPoolManager();
		dumpMetaMemoryLeaks(pMetaAlloc);
		pMetaAlloc->~BlockAllocator();
	}
	void* meta_malloc(size_t bytes, size_t alignment)
	{
		if (bytes > META_BLOCK_EMEM_SIZE * MAX_BLOCK_ELEM_GROUP_N || !pMetaAlloc->try_allocate(bytes)) {
			return pMainPool->do_allocate(bytes, alignment);
		}
		using MemBlock = BlockAllocator::MemBlock;
		bytes += sizeof(MemBlock);
		MemBlock* pBlock = (MemBlock*)pMetaAlloc->do_allocate(bytes, alignment);
		new(pBlock)MemBlock(bytes);
		return (char*)pBlock + sizeof(MemBlock);
	}
	void meta_free(void* ptr)
	{
		if (!ptr) {
			return;
		}
		using MemBlock = BlockAllocator::MemBlock;
		MemBlock* pBlock = (MemBlock*)((char*)ptr - sizeof(MemBlock));
		if (!*pBlock) {
			pMainPool->do_deallocate(ptr);
			return;
		}
		pMetaAlloc->do_deallocate(ptr, pBlock->size, 0);
	}
}
namespace pmr {
	struct MemPoolWrap {
		inline static char AllocData[sizeof(BlockAllocator)];
		char pMemory[sizeof(MemoryPoolManager)];
		bool isMainTread;
		MemPoolWrap() {
			isMainTread = !pMainPool;
			if (isMainTread) {
				pMainPool = (MemoryPoolManager*)&pMemory;
				pMetaAlloc = (BlockAllocator*)&AllocData;
				std::construct_at(pMetaAlloc, MEMORY_BLOCK_SIZE, META_BLOCK_EMEM_SIZE);
				std::atexit(CleanMainMemPool);
			}
			std::construct_at((MemoryPoolManager*)&pMemory);
		}
		~MemPoolWrap() {
			auto pool = (MemoryPoolManager*)&pMemory;
			if (!isMainTread) {//主线程最后析构
				pool->~MemoryPoolManager();
			}
		}
		MemoryPoolManager* operator->() {
			return (MemoryPoolManager*)&pMemory;
		}
	};
}
thread_local pmr::MemPoolWrap MemPool;

MEMALLOC_API void* me_malloc(size_t size)
{
	return MemPool->do_allocate(size, MEMORY_ALIGN_N);
}

MEMALLOC_API void* me_malloc_nothrow(size_t size) noexcept
{
	return MemPool->do_allocate(size, MEMORY_ALIGN_N);
}

MEMALLOC_API void me_free(void* p) noexcept
{
	MemPool->do_deallocate(p);
}
#if (__cplusplus >= 201402L || _MSC_VER >= 1916)
MEMALLOC_API void me_free_size(void* p, size_t size) noexcept
{
	MemPool->do_deallocate(p);
}
#endif
#if (__cplusplus > 201402L || defined(__cpp_aligned_new))
MEMALLOC_API void* me_malloc_aligned(size_t size, size_t alignment)
{
	return MemPool->do_allocate(size, alignment);
}

MEMALLOC_API void* me_malloc_aligned_nothrow(size_t size, size_t alignment) noexcept
{
	return MemPool->do_allocate(size, alignment);
}
MEMALLOC_API void me_free_aligned(void* p, size_t alignment) noexcept
{
	return MemPool->do_deallocate(p);
}
MEMALLOC_API void me_free_size_aligned(void* p, size_t size, size_t alignment) noexcept
{
	MemPool->do_deallocate(p);
}

#endif