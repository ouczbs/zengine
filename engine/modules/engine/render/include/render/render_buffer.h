#pragma once
#define PREALLOCATED_DYNAMIC_BUFFER_PAGES 2
#include "render_api.h"
namespace api {
	struct DynamicBufferDesc {
		uint32_t            size;
		uint32_t            pos;
		uint32_t            drawPos;
		BufferUsage         usage;
		ResourceMemoryUsage memoryUsage;
	};
	struct DynamicBuffer : public DynamicBufferDesc
	{
		struct Page
		{
			uint32_t hash;
			uint32_t frameNumber;
			void*	 pBuffer;
			void*    pAllocation;
			void*    pMappingAddr;
			Page*    next;
		};
		struct PageAllocator
		{
			struct Block
			{
				Page   pages[16];
				Block* next;
			};
			uint32_t pageIndex;
			uint32_t numPages;
			Block*   blocks = nullptr;
		};
		Page*    currentPage;
		Page*    freePages;
		Page*    pendingPages;
		PageAllocator allocator;
		////////////////////////////////////////////////////////////////////////////////////////////////////
		void InitBuffer(uint32_t _size, BufferUsage _usage, ResourceMemoryUsage _memoryUsage)
		{
			pos = 0;
			drawPos = 0;
			size = _size;
			usage = _usage;
			memoryUsage = _memoryUsage;

			pendingPages = nullptr;
			freePages = nullptr;
			allocator.numPages = 0;
			allocator.pageIndex = 0;
			for (uint32_t i = 0; i < PREALLOCATED_DYNAMIC_BUFFER_PAGES; i++)
			{
				Page* page = AllocatePage();
				page->next = freePages;
				freePages = page;
			}

			currentPage = freePages;
			freePages = freePages->next;
		}
		static Page* AllocatePageMemory(PageAllocator& allocator)
		{
			using Block = PageAllocator::Block;
			if (!allocator.blocks || allocator.pageIndex == _countof(Block::pages))
			{
				Block* block = (Block*)xmalloc(sizeof(Block));
				block->next = allocator.blocks;
				allocator.blocks = block;
				allocator.pageIndex = 0;
			}
			allocator.numPages++;
			return allocator.blocks->pages + allocator.pageIndex++;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		Page* AllocatePage()
		{
			Page* page = AllocatePageMemory(allocator);
			memset(page, 0, sizeof(Page));
			BufferDesc desc{};
			desc.size = size;
			desc.usage = usage;
			desc.memoryUsage = memoryUsage;
			RenderAPI::Ptr()->CreateBuffer(desc);
			page->pAllocation = desc.pAllocation;
			page->pBuffer = desc.buffer;
			page->pMappingAddr = desc.pMappingAddr;
			return page;
		}
		void* MapBuffer(uint32_t _size, uint32_t frameNumber, uint32_t safeFrameNumber) {
			if (pos + _size > size)
			{
				// We ran out of space in the current page, get a new one
				// Move the current one to pending and insert a GPU fence
				currentPage->frameNumber = frameNumber;
				currentPage->next = pendingPages;
				pendingPages = currentPage;

				// If there is one free slot get it
				if (freePages != nullptr)
				{
					currentPage = freePages;
					freePages = freePages->next;
				}
				else
				{
					// Move pages already processed by GPU from pending to free
					Page** it = &pendingPages->next;
					while (*it != nullptr)
					{
						if ((*it)->frameNumber > safeFrameNumber)
						{
							it = &((*it)->next);
						}
						else
						{
							// Once we find a processed page, the rest of pages must be also processed
							Page* page = *it;
							*it = nullptr;

							freePages = page;
							break;
						}
					}

					if (freePages != nullptr)
					{
						currentPage = freePages;
						freePages = freePages->next;
					}
					else
					{
						currentPage = AllocatePage();
					}
				}

				pos = 0;
			}

			drawPos = pos;
			pos = pos + _size;
			return (uint8_t*)currentPage->pMappingAddr + drawPos;
		}
	};
}