#include "resource_bundle.h"
#pragma once
namespace api
{
	template<typename Res>
	inline ResourceBundle::ResourceBundle(const RscHandle<Res>& handle)
	{
		Add(handle);
	}
	template<typename T>
	inline void ResourceBundle::Add(RscHandle<T> handle)
	{
		auto& sub_arr = subarrays[ResourceID<T>];
		auto new_ind = sub_arr.index + sub_arr.count++;


		// make space for new resource
		handles.emplace_back();
		std::move_backward(handles.data() + new_ind, handles.data() + handles.size() - 1, handles.data() + handles.size());

		// assign new resource
		handles[new_ind] = handle;

		// push back all subsequent resources
		for (auto& elem : span<sub_array>{ &sub_arr + 1, subarrays.data() + subarrays.size() })
			++elem.index;
	}
	inline ResourceBundle::operator bool()
	{
		return !handles.empty();
	}
	inline span<const GenericResourceHandle> ResourceBundle::GetAll() const
	{
		return span<const GenericResourceHandle>(handles);
	}

	template<typename T>
	inline RscHandle<T> ResourceBundle::Get() const
	{
		auto& subarray = subarrays[ResourceID<T>];
		return subarray.count > 0 ? handles[subarray.index].template AsHandle<T>() : RscHandle<T>();
	}
	template<typename T> span<const GenericResourceHandle> ResourceBundle::GetAll() const {
		auto& subarray = subarrays[ResourceID<T>];
		return span<const GenericResourceHandle>{handles.data() + subarray.index, handles.data() + subarray.index + subarray.count};
	}
}