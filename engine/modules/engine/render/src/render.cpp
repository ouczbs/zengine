#include "render/type.h"
#include "render/asset/ubo.h"
#include <algorithm>
namespace api {
	MaterialResourceBlock MaterialResourceBlock::FromFields(std::span<const refl::FieldPtr> fields)
	{
		uint32_t count = fields.size();
		if (!count) {
			return {};
		}
		uint32_t blockSize = meta_align_size(sizeof(MaterialResource) * count);
		for (auto& field : fields) {
			blockSize += meta_align_size(field.type->size);
		}
		MaterialResource* refResource = (MaterialResource*)xmalloc(blockSize);
		char* pObj = (char*)refResource + meta_align_size(sizeof(MaterialResource) * count);
		MaterialResource* pResource = refResource;
		for (auto& field : fields) {
			new(pResource)MaterialResource{};
			pResource->name = field.name;
			pResource->obj = field.Construct(pObj);
			pResource->isDirty = false;
			ubMetaInfo ub = ubMetaInfo::FromField(field);
			pResource->isUnique = ub.isUnique;
			pObj += meta_align_size(field.type->size);
			pResource++;
		}
		if (count > 1) {
			std::sort(refResource, refResource, [](const MaterialResource& res1, const MaterialResource& res2) {
				return res1.name < res2.name;
				});
		}
		return { count , blockSize , refResource };
	}
}
#ifdef RENDER_API_VAL
#include "renderapi_impl.inl"
#include "window_impl.inl"
#endif // 