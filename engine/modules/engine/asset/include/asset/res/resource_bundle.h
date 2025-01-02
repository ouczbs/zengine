#pragma once
#include "resource_handle.h"
namespace api
{
	struct GenericResourceHandle;
	struct ResourceBundle
	{
		ResourceBundle() = default;
		template<typename Res> // conversion from single resource
		ResourceBundle(const RscHandle<Res>& handle);
		operator bool();
		// will reshuffle vector and invalidate span, but you shouldn't be accessing vector directly anyway so this is ok
		template<typename T> void Add(RscHandle<T> handle); 
		template<typename T> RscHandle<T>    Get() const;    // get a resource from the bundle
		span<const GenericResourceHandle>    GetAll() const; // gets absolutely all resources
		template<typename T> span<const GenericResourceHandle> GetAll() const; // get all resources of one type
	private:
		struct sub_array { short index = 0, count = 0; };

		vector<GenericResourceHandle> handles; // always sorted so that we can simply span
		array<sub_array, ResourceCount> subarrays;
	};
}
#include "resource_bundle.inl"