#include "asset/res/meta_bundle.h"
#include <algorithm>

namespace api
{
    const SerializedMeta* MetaBundle::FetchMeta(const Guid& guid) const
    {
        for (auto& elem : metadatas)
        {
            if (elem.guid == guid)
                return &elem;
        }
        return nullptr;
    }
    bool MetaBundle::operator==(const MetaBundle& other)const
    {
        return metadatas == other.metadatas && includes == other.includes;
    }
    bool MetaBundle::operator!=(const MetaBundle& other)const
    {
        return !(*this == other);
    }
}