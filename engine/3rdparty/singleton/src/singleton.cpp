#include "singleton.h"

namespace singleapi {
    pmr::unsynchronized_pool_resource* MemoryInfo::Pool() {
        static pmr::unsynchronized_pool_resource pool;
        return &pool;
    }
    GlobalManager* GlobalManager::Ptr()
    {
        static GlobalManager globalManager{ MemoryInfo::Pool()};
        return &globalManager;
    }
}
