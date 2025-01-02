#pragma once
#include <mutex>
#include <array>
#include "singleton.h"
#include "frame_allocator.h"
namespace pmr {
	template <class T>
	constexpr inline void hash_combine(size_t& seed, const T& v) noexcept
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	template<typename T1, typename T2, typename Hasher = std::hash<T1>>
	using table = std::pmr::unordered_map<T1, T2, Hasher>;
	using std::pmr::string;
	class Name {
		static constexpr uint32_t IS_USED_MASK = 1u << 29u;
		static constexpr uint32_t STRING_ENTRY_HANDLE_BITS = 29u;
		static constexpr uint32_t STRING_ENTRY_HANDLE_MASK = (1u << 29u) - 1u;
		static constexpr uint32_t MAX_STRING_SIZE = 1023u;
		static constexpr uint32_t MAX_SLOT_POOL_ARRAY_SIZE = 256u;
		static constexpr uint32_t MAX_MEMORY_BLOCK_ARRAY_SIZE = 8192u;
		struct StringEntryHandle
		{
			static constexpr uint16_t  MEMORY_BLOCK_INDEX_MASK = (1u << 13u) - 1u;
		private:
			uint16_t empty3_memoryBlockIndex13;
			uint16_t memoryBlockAlignedOffset16;
		public:
			StringEntryHandle() 
				: empty3_memoryBlockIndex13(0u), memoryBlockAlignedOffset16(0u){}
			StringEntryHandle(uint32_t stringEntryHandleValue)
				: StringEntryHandle(static_cast<uint16_t>(stringEntryHandleValue >> 16)& MEMORY_BLOCK_INDEX_MASK, static_cast<uint16_t>(stringEntryHandleValue)) {}
			StringEntryHandle(uint16_t memoryBlockIndex, uint16_t memoryBlockAlignedOffset)
				: empty3_memoryBlockIndex13(memoryBlockIndex), memoryBlockAlignedOffset16(memoryBlockAlignedOffset) {}
			uint16_t GetMemoryBlockIndex() const {
				return empty3_memoryBlockIndex13;
			}
			uint16_t GetMemoryBlockAlignedOffset() const {
				return memoryBlockAlignedOffset16;
			}
			uint32_t GetStringEntryHandleValue() const {
				return static_cast<uint32_t>(empty3_memoryBlockIndex13) << 16 | memoryBlockAlignedOffset16;
			}
		};
		struct StringEntryHeader
		{
			static constexpr uint16_t SIZE_MASK = (1u << 10u) - 1u;
			static constexpr uint16_t STRING_HASH_PROBE_MASK = ((1u << 6u) - 1u) << 10u;
		private:
			uint16_t stringHashProbe6_size10;
		public:
			void Init(const uint8_t stringHashProbe, const uint16_t size) {
				stringHashProbe6_size10 = (STRING_HASH_PROBE_MASK & (static_cast<uint16_t>(stringHashProbe) << 10u)) | (SIZE_MASK & size);
			}
			uint16_t GetSize() const { return SIZE_MASK & stringHashProbe6_size10; }
			operator uint16_t() { return stringHashProbe6_size10; }
		};
		struct StringEntry
		{
		private:
			StringEntryHeader stringEntryHeader;
			char data[MAX_STRING_SIZE];
		public:
			StringEntryHeader GetStringEntryHeader() const {
				return stringEntryHeader;
			}
			void SetStringEntryHeader(StringEntryHeader _stringEntryHeader) {
				stringEntryHeader = _stringEntryHeader;
			}
			void SetStringData(const char* str, uint16_t size) {
				memcpy(data, str, size);
				data[size] = 0x00;
			}
			uint16_t GetSize()const { return stringEntryHeader.GetSize(); }
			const char* GetData() const { return data; };
		};
		struct HashInfo {
			static constexpr uint32_t SLOT_POOL_INDEX_MASK = (1u << 8u) - 1u;
			static constexpr uint32_t SLOT_HASH_PROBE_MASK = ((1u << 3u) - 1) << 13u;
			static constexpr uint32_t STRING_HASH_PROBE_MASK = (1u << 6u) - 1;
			const char* data;
			StringEntryHeader stringEntryHeader;
			uint16_t slotProbe3_empty21_slotPool8;
			uint32_t slotIndex32;
			HashInfo(std::string_view view) {
				uint64_t hash = string_hash(view);
				uint32_t Hi = static_cast<uint32_t>(hash >> 32);
				uint32_t Lo = static_cast<uint32_t>(hash);

				slotIndex32 = Lo;
				slotProbe3_empty21_slotPool8 = (SLOT_HASH_PROBE_MASK & (Hi >> 16)) | (SLOT_POOL_INDEX_MASK & Hi);
				stringEntryHeader.Init((Hi >> 8u) & STRING_HASH_PROBE_MASK, view.size());
				data = view.data();
			}
			const char* GetData() const {
				return data;
			}
			uint32_t GetSlotIndex() const{
				return slotIndex32;
			}
			StringEntryHeader GetStringEntryHeader() const {
				return stringEntryHeader;
			}
			uint32_t GetSlotPoolIndex() const {
				return SLOT_POOL_INDEX_MASK & slotProbe3_empty21_slotPool8;
			}
			template<uint32_t mask>
			uint32_t GetSlotHashProbe() const
			{
				return mask | (SLOT_HASH_PROBE_MASK & slotProbe3_empty21_slotPool8) << 16;
			}
		};

		struct Slot
		{
			static constexpr uint32_t SLOT_HASH_PROBE_MASK = ((1u << 3u) - 1) << 29u;
			static constexpr uint32_t STRING_ENTRY_HANDLE_MASK = (1u << 29u) - 1u;
			static constexpr uint32_t MEMORY_BLOCK_ALIGNED_OFFSET_MASK = (1u << 16u) - 1u;
			static constexpr uint32_t MEMORY_BLOCK_INDEX_MASK = ((1u << 13u) - 1u);
		private:
			uint32_t slotHashProbe3_stringEntryHandle29;
		public:
			bool IsTargetSlot(HashInfo hashInfo) const;
			bool IsUsed() const{ return (slotHashProbe3_stringEntryHandle29 & IS_USED_MASK) == IS_USED_MASK; }
			uint32_t GetSlotHashProbe() const { return SLOT_HASH_PROBE_MASK & slotHashProbe3_stringEntryHandle29; };
			StringEntryHandle GetStringEntryHandle() const {
				return StringEntryHandle((slotHashProbe3_stringEntryHandle29 >> 16) & MEMORY_BLOCK_INDEX_MASK, slotHashProbe3_stringEntryHandle29 & MEMORY_BLOCK_ALIGNED_OFFSET_MASK);
			}
			uint32_t GetStringEntryHandleValue() const { return slotHashProbe3_stringEntryHandle29 & STRING_ENTRY_HANDLE_BITS; };
			uint32_t GetSlotValue() const { return slotHashProbe3_stringEntryHandle29; };
			void Load(uint32_t slotHashProbeValue,StringEntryHandle stringEntryHandle) {
				slotHashProbe3_stringEntryHandle29 = slotHashProbeValue | stringEntryHandle.GetStringEntryHandleValue();
			}
			void Load(Slot srcSlot) {slotHashProbe3_stringEntryHandle29 = srcSlot.slotHashProbe3_stringEntryHandle29;}
		};
		struct SlotPool
		{
			friend class Name;
			static constexpr double SLOT_POOL_RESIZE_USAGE_RATE = 0.9;
			static constexpr uint32_t SLOT_POOL_INITIALIZE_SIZE = 256u;
		private:
			uint32_t capcity{0};
			uint32_t size{0};
			Slot* slotArray{ nullptr };
			std::mutex mutex;
		public:
			Slot& FindUnusedOrTargetSlot(HashInfo hashInfo);
			Slot& FindUnusedSlot(HashInfo hashInfo);
			void AutoResize();
			void Resize();
		};
		struct StringEntryMemoryManager
		{
			friend class Name;
			static constexpr uint32_t ALIGN_BYTES = 2u;
			static constexpr uint32_t MAX_MEMORY_BLOCK_SIZE = (1u << 16u) * 2u;
		private:
			std::array<SlotPool, MAX_SLOT_POOL_ARRAY_SIZE> slotPoolArray;
			uint16_t currentMemoryBlockIndex;
			uint16_t currentMemoryBlockAlignedCursor;
			char* memoryBlockArray[MAX_MEMORY_BLOCK_ARRAY_SIZE];
			std::mutex mutex;
		public:
			StringEntryMemoryManager();
			void reinitialize();
			StringEntry* GetStringEntry(StringEntryHandle stringEntryHandle) const;
			StringEntryHandle AllocateStringEntry(StringEntryHeader stringEntryHeader, const char* data);
		private:
			void CreateNewMemoryBlock();
		};
		inline static char** g_memory_blocks = nullptr;
		UNIQUER_INLINE_STATIC(StringEntryMemoryManager, stringEntryMemoryManager, "pmr::name::stringEntryMemoryManager")
		uint32_t flag3_memory29;
	public:
		friend class std::hash<Name>;
		Name()noexcept : flag3_memory29(0) {};
		Name(std::string view) noexcept : flag3_memory29(MakeInterned(view)) {};
		Name(std::string_view view) noexcept : flag3_memory29(MakeInterned(view))  {};
		template<size_t N>
		Name(const char(&str)[N]) noexcept : Name(std::string_view(str)) {}
		Name(const char* str)noexcept : Name(std::string_view(str)){};
		constexpr auto operator<=>(const Name& other) const noexcept { return flag3_memory29 <=> other.flag3_memory29; };
		uint32_t MakeInterned(std::string_view view);
		operator uint32_t() const {
			return flag3_memory29;
		}
		operator std::string_view()const {
			return ToStringView();
		}
		operator std::string()const {
			return ToString();
		}
		const char* data()const {
			if (!flag3_memory29)return nullptr;
			const StringEntry* stringEntry = UNIQUER_VAL(stringEntryMemoryManager).GetStringEntry(StringEntryHandle(flag3_memory29));
			return stringEntry->GetData();
		}
		std::string_view ToStringView() const
		{
			if (!flag3_memory29)return "";
			const StringEntry* stringEntry = UNIQUER_VAL(stringEntryMemoryManager).GetStringEntry(StringEntryHandle(flag3_memory29));
			return std::string_view(stringEntry->GetData(), stringEntry->GetSize());
		}

		std::string ToString() const
		{
			if (!flag3_memory29)return "";
			const StringEntry* stringEntry = UNIQUER_VAL(stringEntryMemoryManager).GetStringEntry(StringEntryHandle(flag3_memory29));
			return std::string(stringEntry->GetData(), stringEntry->GetSize());
		}
	};

	class Tag {
	private:
		Name name;
		int number;
	public:
		friend class std::hash<Tag>;
		Tag(Name name, int number): name(name), number(number) {}
		Tag(Name name): name(name), number(0) {}
		Tag(): name(), number(0) {}
		Name getName() const { return name; }
		int  getNumber() const { return number; }
		constexpr auto operator<=>(const Tag& other) const noexcept = default;
	};
}
namespace std {
	template<>
	struct hash<::pmr::Name>
	{
		size_t operator()(const ::pmr::Name& name) const noexcept
		{
			return std::hash<int>{}(name.flag3_memory29);
		}
	};
	template<>
	struct hash<::pmr::Tag>
	{
		size_t operator()(const ::pmr::Tag& tag) const noexcept
		{
			size_t h1 = std::hash<int>{}((uint32_t)tag.name);
			size_t h2 = std::hash<int>{}(tag.number);
			return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
		}
	};
}
#include "name.inl"