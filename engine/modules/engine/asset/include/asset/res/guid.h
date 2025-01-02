#pragma once
#include "refl/pch.h"
#include <objbase.h>  
#include <string>
namespace api
{
	using refl::meta_info;
	using refl::meta_name;
	using std::string_view;
	using std::span;
	struct Guid
	{
		unsigned int   Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];

		Guid() noexcept
			: Data1{ 0 }, Data2{ 0 }, Data3{ 0 }, Data4{ 0,0,0,0,0,0,0,0 }
		{}
		
		USING_OVERLOAD_CTOR(Guid, string_view)
		UFUNCTION({}, ref = USING_CTOR_NAME)
		Guid(string_view str) noexcept : Guid{str.data()} {}
		USING_OVERLOAD_CTOR(Guid, const char*)
		UFUNCTION({}, ref = USING_CTOR_NAME)
		Guid(const std::string& str) noexcept : Guid(str.data()){}
		Guid(const char* str) noexcept
			: Guid{}
		{
			sscanf_s(str,
				"%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
				&Data1, &Data2, &Data3,
				&Data4[0], &Data4[1], &Data4[2], &Data4[3],
				&Data4[4], &Data4[5], &Data4[6], &Data4[7]);

		}
		constexpr Guid(unsigned int a, unsigned short b, unsigned short c, unsigned long long d)
			: Data1{ a }
			, Data2{ b }
			, Data3{ c }
			, Data4{
				(unsigned char)(d >> 56 & 0xFF)
			,	(unsigned char)(d >> 48 & 0xFF)
			,	(unsigned char)(d >> 40 & 0xFF)
			,	(unsigned char)(d >> 32 & 0xFF)
			,	(unsigned char)(d >> 24 & 0xFF)
			,	(unsigned char)(d >> 16 & 0xFF)
			,	(unsigned char)(d >> 8 & 0xFF)
			,	(unsigned char)(d >> 0 & 0xFF)
			}
		{};
		UFUNCTION({})
		std::string ToString()const {
			char guid_cstr[39];
			snprintf(guid_cstr, sizeof(guid_cstr),
				"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
				Data1, Data2, Data3,
				Data4[0], Data4[1], Data4[2], Data4[3],
				Data4[4], Data4[5], Data4[6], Data4[7]);
			return std::string{ guid_cstr };
		}
	public:
		auto operator<=>(const Guid&) const noexcept = default;
		operator bool() const noexcept
		{
			return *reinterpret_cast<const GUID*>(this) != GUID_NULL;
		}
		operator std::string() const
		{
			return ToString();
		}
	public:
		UFUNCTION({})
		static Guid Make()
		{
			Guid guid;
			const auto res = CoCreateGuid((GUID*)&guid);
			return guid;
		}
	};
}
#include "guid.inl"