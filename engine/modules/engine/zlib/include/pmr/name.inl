#include "name.h"
constexpr inline size_t meta_align_size(size_t size, size_t alignment = 8) {
	return (size + (alignment - 1)) & ~(alignment - 1);
}
XMALLOC_API void* xmalloc(size_t size);
XMALLOC_API void  xfree(void* p) noexcept;
namespace pmr {
	inline bool Name::Slot::IsTargetSlot(HashInfo hashInfo) const
	{
		if (GetSlotHashProbe() == hashInfo.GetSlotHashProbe<IS_USED_MASK>())
		{
			const StringEntry* stringEntry = UNIQUER_VAL(stringEntryMemoryManager).GetStringEntry(GetStringEntryHandle());
			auto entryHeader = stringEntry->GetStringEntryHeader();
			if (entryHeader == hashInfo.GetStringEntryHeader() && 0 == strncmp(stringEntry->GetData(), hashInfo.GetData(), entryHeader.GetSize()))
			{
				return true;
			}
		}
		return false;
	}
	inline Name::Slot& Name::SlotPool::FindUnusedOrTargetSlot(HashInfo hashInfo)
	{
		const uint32_t SLOT_POOL_CAPCITY_MASK = capcity - 1;
		for (uint32_t slotIndex = hashInfo.GetSlotIndex() & SLOT_POOL_CAPCITY_MASK; true; slotIndex = (slotIndex + 1) & SLOT_POOL_CAPCITY_MASK)
		{
			Slot& slot = slotArray[slotIndex];

			if (!slot.IsUsed() || slot.IsTargetSlot(hashInfo))
			{
				return slot;
			}
		}
	}
	inline Name::Slot& Name::SlotPool::FindUnusedSlot(HashInfo hashInfo)
	{
		const uint32_t SLOT_POOL_CAPCITY_MASK = capcity - 1;
		for (uint32_t slotIndex = hashInfo.GetSlotIndex() & SLOT_POOL_CAPCITY_MASK; true; slotIndex = (slotIndex + 1) & SLOT_POOL_CAPCITY_MASK)
		{
			Slot& slot = slotArray[slotIndex];

			if (!slot.IsUsed())
			{
				return slot;
			}
		}
	}
	inline void Name::SlotPool::AutoResize()
	{
		if (size > SLOT_POOL_RESIZE_USAGE_RATE * capcity)
		{
			Resize();
		}
	}
	inline void Name::SlotPool::Resize()
	{
		uint32_t oldCapcity = capcity;
		Slot* oldSlotArray = slotArray;

		capcity = oldCapcity * 2;
		slotArray = reinterpret_cast<Slot*>(xmalloc(capcity * sizeof(Slot)));
		//std::memset(slotArray, 0, capcity * sizeof(Slot));
		auto& stringEntryMemoryManager = UNIQUER_VAL(stringEntryMemoryManager);
		for (uint32_t slotIndex = 0; slotIndex < size; slotIndex++)
		{
			const Name::Slot& oldSlot = oldSlotArray[slotIndex];

			StringEntry* stringEntry = stringEntryMemoryManager.GetStringEntry(oldSlotArray[slotIndex].GetStringEntryHandle());
			const HashInfo hashInfo(std::string_view(stringEntry->GetData(), stringEntry->GetSize()));

			Name::Slot& slot = FindUnusedSlot(hashInfo);

			slot.Load(oldSlot);
		}

		std::free(oldSlotArray);
	}
	inline Name::StringEntryMemoryManager::StringEntryMemoryManager()
	{
		for (auto& slotPool : slotPoolArray)
		{
			slotPool.size = 0;
			slotPool.capcity = SlotPool::SLOT_POOL_INITIALIZE_SIZE;
			slotPool.slotArray = (Slot*)(xmalloc(SlotPool::SLOT_POOL_INITIALIZE_SIZE * sizeof(Slot)));
			std::memset(slotPool.slotArray, 0, SlotPool::SLOT_POOL_INITIALIZE_SIZE);
		}
		currentMemoryBlockIndex = 0;
		currentMemoryBlockAlignedCursor = 0;
		memoryBlockArray[0] = (char*)(xmalloc(MAX_MEMORY_BLOCK_SIZE));
		std::memset(memoryBlockArray[0], 0, MAX_MEMORY_BLOCK_SIZE);
	}
	inline void Name::StringEntryMemoryManager::reinitialize()
	{
		g_memory_blocks = memoryBlockArray;
	}
	inline Name::StringEntry* Name::StringEntryMemoryManager::GetStringEntry(StringEntryHandle stringEntryHandle) const
	{
		return reinterpret_cast<Name::StringEntry*>(memoryBlockArray[stringEntryHandle.GetMemoryBlockIndex()] + stringEntryHandle.GetMemoryBlockAlignedOffset() * ALIGN_BYTES);
	}
	inline Name::StringEntryHandle Name::StringEntryMemoryManager::AllocateStringEntry(StringEntryHeader stringEntryHeader, const char* data)
	{
		const uint32_t size = meta_align_size(sizeof(StringEntryHeader) + stringEntryHeader.GetSize() + 1, ALIGN_BYTES);
		const uint32_t alignedSize = size >> 1;

		StringEntryHandle stringEntryHandle{};
		{
			std::unique_lock<std::mutex> lock(mutex);

			if (((static_cast<uint32_t>(currentMemoryBlockAlignedCursor) + alignedSize) << 1) >= MAX_MEMORY_BLOCK_SIZE)
			{
				CreateNewMemoryBlock();
			}

			stringEntryHandle = StringEntryHandle(currentMemoryBlockIndex, currentMemoryBlockAlignedCursor);
			currentMemoryBlockAlignedCursor += alignedSize;
		}

		StringEntry* stringEntry = GetStringEntry(stringEntryHandle);
		stringEntry->SetStringEntryHeader(stringEntryHeader);
		stringEntry->SetStringData(data, stringEntryHeader.GetSize());
		return stringEntryHandle;
	}
	inline void Name::StringEntryMemoryManager::CreateNewMemoryBlock()
	{
		if (currentMemoryBlockIndex >= (MAX_MEMORY_BLOCK_ARRAY_SIZE - 1))
			throw std::runtime_error("Interned string only supports a maximum of 1GB memory.");

		currentMemoryBlockIndex += 1;
		currentMemoryBlockAlignedCursor = 0;
		memoryBlockArray[currentMemoryBlockIndex] = (char*)(xmalloc(MAX_MEMORY_BLOCK_SIZE));
		std::memset(memoryBlockArray[currentMemoryBlockIndex], 0, MAX_MEMORY_BLOCK_SIZE);
	}
	inline uint32_t Name::MakeInterned(std::string_view view)
	{
		if (view.empty()) {
			return 0u;
		}
		HashInfo hashInfo(view);
		auto& stringEntryMemoryManager = UNIQUER_VAL(stringEntryMemoryManager);
		auto& slotPool = stringEntryMemoryManager.slotPoolArray[hashInfo.GetSlotPoolIndex()];
		uint32_t slotValue = 0u;
		{
			std::unique_lock<std::mutex> lock(slotPool.mutex);
			slotPool.AutoResize();
			Name::Slot& slot = slotPool.FindUnusedOrTargetSlot(hashInfo);
			if (!slot.IsUsed())
			{
				slotPool.size++;
				StringEntryHandle stringEntryHandle = stringEntryMemoryManager.AllocateStringEntry(hashInfo.GetStringEntryHeader(), hashInfo.GetData());
				slot.Load(hashInfo.GetSlotHashProbe<IS_USED_MASK>(), stringEntryHandle);
			}
			slotValue = slot.GetSlotValue();
		}
		return slotValue;
	}
}