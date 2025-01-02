#pragma once
#if !defined(__cppast)
#define __cppast(...)
#endif
#define __Meta(...) __cppast(Meta=__VA_ARGS__)
#define UPROPERTY(...) __Meta(__VA_ARGS__)
#define UFUNCTION(...) __Meta(__VA_ARGS__)

#define __glMeta(...) __cppast(glMeta=__VA_ARGS__)
#define UPROPERTY_gl(...) __glMeta(__VA_ARGS__)
#define UFUNCTION_gl(...) __glMeta(__VA_ARGS__)

#define __vkMeta(...) __cppast(vkMeta=__VA_ARGS__)
#define UPROPERTY_vk(...) __vkMeta(__VA_ARGS__)
#define UFUNCTION_vk(...) __vkMeta(__VA_ARGS__)

#define __dxMeta(...) __cppast(dxMeta=__VA_ARGS__)
#define UPROPERTY_dx(...) __dxMeta(__VA_ARGS__)
#define UFUNCTION_dx(...) __dxMeta(__VA_ARGS__)

// 辅助宏，用于实际拼接
#define CONCATENATE(arg1, arg2)   CONCATENATE_IMPL(arg1, arg2)
#define CONCATENATE_IMPL(arg1, arg2)  arg1##arg2
#define MY_UNIQUE_NAME(base) CONCATENATE(base, __LINE__)

#define TOSTRING_IMPL(X) #X
#define TOSTRING(X) TOSTRING_IMPL(X)

#define USING_CTOR_NAME MY_UNIQUE_NAME(__Ctor)
#define USING_FUNC_NAME MY_UNIQUE_NAME(__Func)

#define USING_OVERLOAD_CTOR(Class, ...) using USING_CTOR_NAME = Class(*)(__VA_ARGS__);
#define USING_OVERLOAD_FUNC(R, ...) using USING_FUNC_NAME = R(*)(__VA_ARGS__);
#define USING_OVERLOAD_CLASS_FUNC(R, Class, ...) using USING_FUNC_NAME = R(Class::*)(__VA_ARGS__);

#define GENERATED_BODY() template <typename T, size_t hash>\
friend class gen::MetaImpl;
/*
struct vec3{
	USING_OVERLOAD_CTOR(vec3)
	UFUNCTION({},ref = USING_CTOR_NAME)
	vec3(){}
}
*/