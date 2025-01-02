#pragma once
namespace api{
	template<typename Res>
	inline Guid Resource<Res>::GetGuid() const
	{
		return GetHandle().guid;
	}
	struct GenericResourceHandle
		: variant_wrap_t<tuple_to_variant_t<Resources>, RscHandle>
	{
	private:
		using Base = variant_wrap_t<tuple_to_variant_t<Resources>, RscHandle>;
	public:
		using Base::Base;
		using Base::operator=;
		template<typename T>
		GenericResourceHandle(RscHandle<T> handle) : Base(RscHandle<typename T::BaseResource>{handle}) {}
		GenericResourceHandle(string_view meta_name, Guid guid);
		template<typename T> RscHandle<T> AsHandle() const {
			return std::get<ResourceID<T>>(*this);
		}
		Guid   guid() const;
		size_t resource_id() const;
	};
}
// hashtable support
namespace std
{
	template<typename Res>
	struct hash <api::RscHandle<Res>>
	{
		size_t operator()(const api::RscHandle<Res>& res) const noexcept
		{
			return std::hash<api::Guid>()(res.guid);
		}
	};
}
namespace refl::detail {
	template<typename T>
	struct real_type<api::RscHandle<T>> {
		using type = api::RscHandleBase;
	};
}