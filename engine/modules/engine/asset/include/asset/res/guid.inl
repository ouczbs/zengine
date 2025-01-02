#pragma once
namespace std
{
	template<> struct hash<api::Guid>
	{
		size_t operator()(const api::Guid& guid) const noexcept
		{
			const size_t* p = reinterpret_cast<const size_t*>(&guid);
			size_t seed = 0;
			::pmr::hash_combine(seed, p[0]);
			::pmr::hash_combine(seed, p[1]);
			return seed;
		}
	};
}