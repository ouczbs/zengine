#pragma once
#include <memory_resource>
#include <unordered_map>
namespace singleapi {
    namespace pmr {
        using std::pmr::unsynchronized_pool_resource;
        using std::pmr::unordered_map;
    };
    template <class T>
    concept is_unique_wrap_t = requires(T* t) { typename T::UniqueType; { t->Ptr() } -> std::same_as<typename T::UniqueType*>; };
    template <class T>
    concept is_reinitialize_t = requires(T * t) { { t->reinitialize() }; };
    struct MemoryInfo {
        void* data;
        bool  isAlive;
        static pmr::unsynchronized_pool_resource* Pool();
        static void* Allocate(size_t size, size_t align) {
            return Pool()->allocate(size, align);
        }
    };
    class SINGLETON_API GlobalManager {
    public:
        MemoryInfo GetInstance(size_t hash, size_t size, size_t align = 8) {
            auto it = instances.find(hash);
            if (it == instances.end()) {
                void* data = MemoryInfo::Allocate(size, align);
                instances.emplace(hash, MemoryInfo{data, true});
                return MemoryInfo{ data, false };
            }
            bool isAlive = it->second.isAlive;
            it->second.isAlive = true;
            return MemoryInfo{ it->second.data, isAlive };
        }
        bool KillInstance(size_t hash) {
            bool isAlive = false;
            auto it = instances.find(hash);
            if (it != instances.end()) {
                isAlive = it->second.isAlive;
                it->second.isAlive = false;
            }
            return isAlive;
        }
        GlobalManager(pmr::unsynchronized_pool_resource* pool) : instances{pool} {}
        ~GlobalManager() {}
        static GlobalManager* Ptr();
    private:
        pmr::unordered_map<size_t, MemoryInfo> instances;
    };
}
template <typename T, size_t hash = 0>
class UniquePtr {
protected:
    inline static T* ms_Singleton = nullptr;
public:
    template<typename... Args>
    UniquePtr(Args&&... args) {
        using namespace singleapi;
        MemoryInfo info{};
        if constexpr (hash == 0) {
            size_t tHash = typeid(T).hash_code();
            info = GlobalManager::Ptr()->GetInstance(tHash, sizeof(T));
        }
        else {
            info = GlobalManager::Ptr()->GetInstance(hash, sizeof(T));
        }
        if (info.isAlive) {
            ms_Singleton = (T*)info.data;
        }
        else {
            ms_Singleton = new(info.data)T(std::forward<Args>(args)...);
        }
        if constexpr (is_reinitialize_t<T>) {
            ms_Singleton->reinitialize();
        }
    }
    ~UniquePtr() {
        using namespace singleapi;
        bool isAlive = false;
        if constexpr (hash == 0) {
            size_t tHash = typeid(T).hash_code();
            isAlive = GlobalManager::Ptr()->KillInstance(tHash);
        }
        else {
            isAlive = GlobalManager::Ptr()->KillInstance(hash);
        }
        if (ms_Singleton && isAlive) {
            ms_Singleton->~T();
        }
    }
    static T* Ptr(void) {
        return ms_Singleton;
    }
};
template <singleapi::is_unique_wrap_t T, size_t hash>
class UniquePtr<T, hash> {
protected:
    using U = T::UniqueType;
    inline static T* ms_Singleton = nullptr;
public:
    template<typename... Args>
    UniquePtr(Args&&... args) {
        using namespace singleapi;
        MemoryInfo info{};
        if constexpr (hash == 0) {
            size_t tHash = typeid(T).hash_code();
            info = GlobalManager::Ptr()->GetInstance(tHash, sizeof(T));
        }
        else {
            info = GlobalManager::Ptr()->GetInstance(hash, sizeof(T));
        }
        if (info.isAlive) {
            ms_Singleton = (T*)info.data;
        }
        else {
            ms_Singleton = new(info.data)T(std::forward<Args>(args)...);
        }
        if constexpr (is_reinitialize_t<T>) {
            ms_Singleton->reinitialize();
        }
    }
    ~UniquePtr() {
        using namespace singleapi;
        bool isAlive = false;
        if constexpr (hash == 0) {
            size_t tHash = typeid(T).hash_code();
            isAlive = GlobalManager::Ptr()->KillInstance(tHash);
        }
        else {
            isAlive = GlobalManager::Ptr()->KillInstance(hash);
        }
        if (ms_Singleton && isAlive) {
            ms_Singleton->~T();
        }
    }
    static U* Ptr(void) {
        return ms_Singleton->Ptr();
    }
};
constexpr inline size_t string_hash(std::string_view str) noexcept
{
    constexpr size_t fnv_offset_basis = 0xcbf29ce484222325;
    constexpr size_t fnv_prime = 0x100000001b3;

    auto hash = fnv_offset_basis;
    for (auto& elem : str)
    {
        hash *= fnv_prime;
        hash ^= elem;
    }
    hash *= fnv_prime;
    hash ^= 0;

    return hash;
}
#define SINGLETON_IMPL(T) \
protected:\
    static T* ms_Singleton;\
public:\
    static T* Ptr(void);
#define SINGLETON_DEFINE(T) T* T::ms_Singleton = nullptr;\
    T* T::Ptr(){ return ms_Singleton; }

#define SINGLETON_PTR() ms_Singleton = this;

#define UNIQUER_INLINE_STATIC(cls, name, hash) inline static UniquePtr<cls, string_hash(hash)> name##Ptr;
#define UNIQUER_INLINE(cls, name, hash) inline UniquePtr<cls, string_hash(hash)> name##Ptr;
#define UNIQUER_STATIC(cls, name, hash) static UniquePtr<cls, string_hash(hash)> name##Ptr;
#define UNIQUER_VAL(name) (*name##Ptr.Ptr())
#define UNIQUER_PTR(name) name##Ptr.Ptr()