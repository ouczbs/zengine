#include "asset/res/resource_handle.h"
namespace api
{
	GenericResourceHandle::GenericResourceHandle(string_view meta_name, Guid guid)
	{
		
	}
	Guid GenericResourceHandle::guid() const
	{
		return std::visit([](const auto& handle) { return handle.guid; }, *this);
	}

	size_t GenericResourceHandle::resource_id() const
	{
		return std::visit([](const auto& handle) { return handle.RscID(); }, *this);
	}
}
