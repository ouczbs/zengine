
#pragma once
#include "pmr/name.h"
#include "meta/variant.h"
#include "meta/tuple.h"
#include "meta/comparable.h"
#include <vector>
#include <optional>
namespace api {
	using namespace meta;
	using std::vector;
	using std::array;
	using pmr::table;
	using pmr::Name;
	template<typename T>
	using opt = std::optional<T>;
	template<bool val>
	using sfinae = std::enable_if_t<val>;
}