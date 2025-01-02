#pragma once
#include "type.h"
namespace api {
	class Guid;
	template<typename Res>
	struct RscHandle;
	template<typename Res>
	class Resource
	{
	public:
		using BaseResource = Res;
		Guid GetGuid()const;
		RscHandle<Res> GetHandle() const { return mHandle; }
		Resource() = default;

		pmr::Name Name() const { return mName; }
		void      Name(pmr::Name n) { mName = n; }
	private:
		RscHandle<Res>    mHandle;
		pmr::Name         mName;
		friend class RscHandle<Res>;
		friend class ResourceSystem;
	};
	using Resources = std::tuple<
		class ShaderProgram
		, class Asset
		, class Texture
	>;
	template<typename Resource>
	concept is_resource_v = requires { typename Resource::BaseResource; };

	template<typename Resource, typename Enable = void>
	struct ResourceID_impl {
		static constexpr auto value() { return index_in_tuple_v<Resource, Resources>; }
	};

	template<typename Resource>
	struct ResourceID_impl<Resource, std::enable_if_t<is_resource_v<Resource>>> {
		static constexpr auto value() { return index_in_tuple_v<typename Resource::BaseResource, Resources>; }
	};

	template<typename Resource> 
	constexpr auto ResourceID = ResourceID_impl<Resource>::value();
	
	constexpr auto ResourceCount = std::tuple_size_v<Resources>;
}