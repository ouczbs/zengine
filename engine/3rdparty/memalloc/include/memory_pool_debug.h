#pragma once
#include "memory_pool.h"
namespace pmr {
	inline void dumpMemoryPoolLeaks(MemoryPoolManager* memPool) {
		auto& mAllocators = memPool->mAllocators;
		for (uint32_t i = 0; i < MAX_BLOCK_ELEM_SIZE_N; i++) {
			auto& pAlloc = mAllocators[i];
			if (pAlloc && !pAlloc->empty()) {
				dumpMetaMemoryLeaks(pAlloc.alloc);
			}
		}
	}
	inline void dumpMetaMemoryLeaks(BlockAllocator* metaAlloc) {
		if (metaAlloc->empty()) {
			return;
		}
		uint64_t* bitmap = (uint64_t*)metaAlloc->m_bitmap;
		size_t   use_count = (metaAlloc->m_head + 63 )/ 64;
		for (size_t i = 0; i < use_count; i++) {
			uint64_t chunk = bitmap[i];
			while (chunk != 0) {
				int offset = __builtin_ctz(chunk);  // 找到最低有效位 "1" 的位置
				void* ptr = metaAlloc->m_data + metaAlloc->m_block_size * (i * 64 + 63 - offset);
				MemoryPoolManager::MemBlock* pBlock = (MemoryPoolManager::MemBlock*)((char*)ptr - sizeof(MemoryPoolManager::MemBlock));
				if (*pBlock) {
					metaAlloc->do_deallocate(pBlock, pBlock->size, 0);
					chunk = bitmap[i];
				}
				else {
					chunk &= chunk - 1;  // 清除最低有效位的“1”
				}
			}
		}
	}
}