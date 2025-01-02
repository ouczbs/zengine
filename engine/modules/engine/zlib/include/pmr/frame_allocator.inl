namespace pmr {
	inline void* try_allocate(FrameAllocator& allac, size_t bytes, size_t alignment)
	{
		if (allac.capacity - allac.offset > bytes) {
			return allac.do_allocate(bytes, alignment);
		}
		return nullptr;
	}

	inline void* FrameAllocator::do_allocate(size_t bytes, size_t alignment)
	{
		size_t space = capacity - offset;
		void* ptr = buffer + offset;
		if (bytes > capacity) {
			throw std::bad_alloc();
		}
		if (std::align(alignment, bytes, ptr, space)) {
			offset = capacity - space + bytes;
			return ptr;
		}
		throw std::bad_alloc();
	}
	inline void* FrameAllocatorPool::allocate(size_t bytes, size_t alignment)
	{
		for (auto& alllocator : allocators) {
			if (auto ptr = try_allocate(alllocator, bytes, alignment)) {
				return ptr;
			}
		}
		// 如果所有现有的分配器都没有足够的空间，则创建一个新的分配器
		auto& it = allocators.emplace_back(allocatorSize);
		return it.allocate(bytes, alignment);
	}
	inline void FrameAllocatorPool::reset()
	{
		size_t count = 0;
		for (auto& allocator : allocators) {
			if (!allocator.empty()) {
				allocator.reset();
				count++;
			}
		}
		count = count > 0 ? count : 1;
		allocators.erase(allocators.begin() + count, allocators.end());
	}
	inline FrameAllocator& FrameAllocator::operator=(FrameAllocator&& o)noexcept
	{
		std::construct_at(this, std::forward<FrameAllocator&&>(o));
		return *this;
	}
#ifdef ZLIB_API_VAL
	inline FrameAllocator::FrameAllocator(FrameAllocator&& o) noexcept :buffer(o.buffer), capacity(o.capacity), offset(o.offset)
	{
		o.move_clear();
	}
	inline FrameAllocator::FrameAllocator(size_t size)noexcept
	{
		buffer = new char[size];
		capacity = size;
		offset = 0;
	}
	inline FrameAllocator::~FrameAllocator()noexcept
	{
		if (buffer)
			delete[] buffer;
	}
};
	ZLIB_API inline pmr::FrameAllocatorPool* MetaGlobalPool() {
		static pmr::FrameAllocatorPool* metaPool = new pmr::FrameAllocatorPool();
		return metaPool;
	}
	ZLIB_API inline pmr::FrameAllocatorPool* GlobalPool() {
		static pmr::FrameAllocatorPool* globalPool = new pmr::FrameAllocatorPool();
		return globalPool;
	}
	ZLIB_API inline pmr::FrameAllocatorPool* FramePool() {
		static pmr::FrameAllocatorPool* framePool = new pmr::FrameAllocatorPool();
		return framePool;
	}
namespace pmr{
#endif // ZLIB_API_VAL
};