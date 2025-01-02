#pragma once
#include "material.h"
#include "vertex.h"
#include "refl/std/parray.h"
namespace api {
	using refl::parray;
	class Mesh : public Asset {
	protected:
		MaterialInstance mMaterial;
		parray<Vertex>	 mVertices;
		vector<uint32_t> mIndices;
	public:
		Mesh();
		template<Vertexs T>
		Mesh(vector<T>& vertices, vector<uint32_t>& indices);
		void BeginLoad() {};
	public:
		Guid GetShaderGuid() {
			return mMaterial.GetShaderGuid();
		}
		MaterialInstance& GetMaterialInstance() {
			return mMaterial;
		}
		void SetMaterial(RscHandle<Material> material) {
			mMaterial.SetMaterial(material);
		}
		RscHandle<Material> GetMaterial() {
			return mMaterial.GetMaterial();
		}
		parray<Vertex>& GetVertices() {
			return mVertices;
		}
		vector<uint32_t>& GetIndices() {
			return mIndices;
		}
	};
	template<Vertexs T>
	inline Mesh::Mesh(vector<T>& vertices, vector<uint32_t>& indices)
		: Asset(meta_info<Mesh>()), mVertices(vertices), mIndices(indices)
	{
	}
};