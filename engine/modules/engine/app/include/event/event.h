#pragma once
#include "pmr/name.h"
#include <functional>
#include <unordered_map>
namespace api{
    using pmr::Name;
    //* Utility functions to create std::functions without std::placeholder
    template<class>
    class Event {};
    template <class R, class... Args>
    class Event<R(Args...)> {
    private:
        Name name;
        using Delegate = std::function<R(Args...)>;
        std::unordered_map<const void*, Delegate> listeners;
        template<typename Type>
        static constexpr auto add_const(R(Type::* ptr)(Args...)) {
            using MethodType = R(Type::*)(Args...)const;
            return (MethodType)ptr;
        }
        template<class Type>
        static std::function<R(Args...)> EasyBind(R(Type::* func)(Args... args) const, const Type* invoker) {
            return [=](auto&&... args) {
                return (invoker->*func)(std::forward<decltype(args)>(args)...);
            };
        }
    public:
        Event(Name name) : name(name){};
        template <class Invoker, class Type>
        void Subscribe(R(Type::*func)(Args... args),const Invoker* invoker) {
            Subscribe<Invoker, Type>(add_const(func), invoker);
        }
        template <class Invoker, class Type>
        void Subscribe(R (Type::*func)(Args... args) const, const Invoker* invoker) { 
            auto found{ listeners.find(invoker) };
            if (found == listeners.end()) {
                listeners.emplace(invoker, EasyBind(func, invoker));
            }
        }
        void Subscribe(Name key,const std::function<R(Args...)>& func) {
            auto id = key.data();
            auto found{listeners.find(id)};
            if (found == listeners.end()) {
                listeners.emplace(id, func);
            }
        }
        template <class Invoker>
        void Unsubscribe(const Invoker* invoker) {    
            auto found {listeners.find(invoker)};
            if (found != listeners.end()) {
                listeners.erase(found);
            }
        }
        void Unsubscribe(Name key) {
            auto id = key.data();
            auto found{listeners.find(id)};
            if (found != listeners.end()) {
                listeners.erase(found);
            }
        }
        void Invoke(Args... args) {
            for (auto& func : listeners) {
                func.second(std::forward<decltype(args)>(args)...);
            }
        }
        void operator()(Args... args) {
            Invoke(std::forward<decltype(args)>(args)...);
        }
    };
}