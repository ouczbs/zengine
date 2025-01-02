#pragma once
#include "any.h"
#include "singleton.h"
namespace refl {
	using UClassPair = std::pair<const UClass*, const UClass*>;
	// 自定义哈希函数
	struct UClassPairHash {
		std::size_t operator()(const UClassPair& pair) const {
			std::hash<const UClass*> ptr_hash;
			return ptr_hash(pair.first) ^ (ptr_hash(pair.second) << 1);
		}
	};
	struct UClassPairEqual {
		bool operator()(const UClassPair& lhs, const UClassPair& rhs) const {
			return lhs.first == rhs.first && lhs.second == rhs.second;
		}
	};
	using ConvertFunc = bool (*)(Any& to, const Any& from);
	using ConvertMap = std::unordered_map<UClassPair, ConvertFunc, UClassPairHash, UClassPairEqual>;
	struct ConvertMapWrap {
		using UniqueType = ConvertMap;
		UniqueType table;
		ConvertMapWrap();
		UniqueType* Ptr() {
			return &table;
		}
	};
	class Convert {
	protected:
		friend class ConvertMapWrap;
		static ConvertMap BuildConvertMap();
		template<typename From, typename To>
		static bool ConvertTo(Any& to, const Any& from);
		static ConvertMap& GetConvertMap();
	public:
		static bool Construct(Any& to, const Any& from);
	};
}