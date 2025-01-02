#include "convert.h"
namespace refl {
	inline ConvertMapWrap::ConvertMapWrap() : table(Convert::BuildConvertMap()){}
	template<typename From, typename To>
	inline bool refl::Convert::ConvertTo(Any& to, const Any& from)
	{
		if constexpr (std::is_same_v<To, std::string>) {
			if constexpr (std::is_same_v<From, char>) {
				std::construct_at(to.CastTo<To*>(), from.CastTo<From*>());
				return true;
			}
			return false;
		}
		else if constexpr (std::is_arithmetic_v<To>) {
			if constexpr (std::is_arithmetic_v<From>) {
				*to.CastTo<To*>() = (To)*from.CastTo<From*>();
				return true;
			}
			return false;
		}
		return false;
	}
	inline ConvertMap Convert::BuildConvertMap()
	{
		ConvertMap convertMap;
#define RegisterToFrom(To, From) convertMap.emplace(std::make_pair(meta_info<To>(), meta_info<From>()), &Convert::ConvertTo<To, From>);\
		convertMap.emplace(std::make_pair(meta_info<From>(), meta_info<To>()), &Convert::ConvertTo<From, To>)

		RegisterToFrom(int, uint32_t);
		RegisterToFrom(int, uint16_t);
		RegisterToFrom(int, float);
		RegisterToFrom(int, double);
		RegisterToFrom(float, uint32_t);
		RegisterToFrom(float, double);
		RegisterToFrom(std::string, char);
#undef RegisterToFrom
		return convertMap;
	}
	inline ConvertMap& Convert::GetConvertMap()
	{
		UNIQUER_STATIC(ConvertMapWrap, ConvertMap, "refl::convert.ConvertMap")
		return UNIQUER_VAL(ConvertMap);
	}
	inline bool Convert::Construct(Any& to, const Any& from)
	{
		if (to.Construct(from))
			return true;
		auto& ConvertMap = GetConvertMap();
		auto it = ConvertMap.find(std::make_pair(to.cls, from.cls));
		if (it != ConvertMap.end()) {
			return it->second(to, from);
		}
		return false;
	}
}