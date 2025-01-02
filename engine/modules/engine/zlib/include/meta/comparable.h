#pragma once
#include <utility>
namespace meta
{
	template<typename T>
	struct comparable
	{
		bool operator>(const T&)  const;
		bool operator<=(const T&) const;
		bool operator>=(const T&) const;
		bool operator==(const T&) const;
		bool operator!=(const T&) const;
	private:
		T& me();
		const T& me() const;
	};
}
#include "comparable.inl"