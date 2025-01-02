#include "field.h"
namespace refl{
	template<typename T, typename R, typename ...Args>
	consteval auto fetch_method_t(R(T::*)(Args...)) {
		using MethodType = R(*)(const void*, Args...);
		return MethodType{ nullptr };
	}
	template<typename R, typename ...Args>
	consteval auto fetch_method_t(R(*)(Args...)) {
		using MethodType = R(*)(Args...);
		return MethodType{ nullptr };
	}
    template<typename Func, typename ...Args>
    inline auto FieldPtr::Call(Func func, Args && ...args) const
    {
        using MemberFunc = decltype(fetch_method_t(func));
        MemberFunc fptr = (MemberFunc)data.method.fptr;
        return fptr(std::forward<Args>(args)...);
    }
	template<bool IsSafeMemory>
	inline bool FieldPtr::Invoke(span<Any> ArgsList) const
	{
		auto Call = type->vtable.Call();
		if (Call) {
			span<const UClass*> params = type->GetParams();
			size_t paramsSize = params.size();
			size_t argsSize = ArgsList.size();
			if (argsSize > paramsSize) {
				ArgsList = span<Any>{ ArgsList.data(), paramsSize };
			}
			if constexpr (IsSafeMemory) {
				if (argsSize < paramsSize && flag & FIELD_METHOD_VALUE_FLAG) {
					auto value = data.method.value;
					if (argsSize + value.size() >= paramsSize) {
						ArgsList = span<Any>{ ArgsList.data(), paramsSize };
						std::copy(value.begin(), value.end(), &ArgsList[argsSize]);
						argsSize = paramsSize;
					}
				}
				if (argsSize < paramsSize) {
					return false;
				}
				for (size_t i = 1; i < paramsSize; i++) {
					if (ArgsList[i].cls != params[i] && ArgsList[i].Check(params[i])) {
						return false;
					}
				}
				Call(this, ArgsList);
			}
			else {
				std::array<Any, MAX_ARGS_LENGTH> ArgsArray{};
				if (argsSize < paramsSize && flag & FIELD_METHOD_VALUE_FLAG) {
					auto value = data.method.value;
					if (argsSize + value.size() >= paramsSize) {
						std::copy(ArgsList.begin(), ArgsList.end(), ArgsArray.data());
						ArgsList = span<Any>{ ArgsArray.data(), paramsSize };
						std::copy(value.begin(), value.end(), &ArgsList[argsSize]);
						argsSize = paramsSize;
					}
				}
				if (argsSize < paramsSize) {
					return false;
				}
				for (size_t i = 1; i < paramsSize; i++) {
					if (ArgsList[i].cls != params[i] && ArgsList[i].Check(params[i])) {
						return false;
					}
				}
				Call(this, ArgsList);
			}
		}
		return Call;
	}
}