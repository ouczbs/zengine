#include "render/type.h"
#include "vertex.h"
namespace api{
    struct ubMetaInfo {
		uint32_t set{0};
		uint32_t binding{0};
		bool     isUnique{false};
		static ubMetaInfo FromField(const refl::FieldPtr& field) {
			refl::Any meta = field.GetMeta();
			if (meta) {
				return meta.CastTo<ubMetaInfo>();
			}
			return {};
		}
	};
	struct ubSimple {
		UPROPERTY({}, api::ubMetaInfo{ .set = 0, .binding = 0 , .isUnique = true})
		Vector4 uColor;
	};
}
#include ".render/ubo_gen.inl"