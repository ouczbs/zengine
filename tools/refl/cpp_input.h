#pragma once
//#include "material.h"
//#include "vertex.h"
//#include "refl/std/parray.h"
namespace engineapi {
	using refl::parray;
	class Mesh : public Asset {
	protected:
		REFL_FRIEND(Mesh)
			UPROPERTY()
		RscHandle<Material> mMaterial;
		parray<Vertex> mVertices;
		vector<uint32_t> mIndices;
	public:
		template<typename T>
			requires std::is_base_of_v<Vertex, T>
		Mesh(vector<T>& vertices, vector<uint32_t>& indices);
		void BeginLoad();

	public:
		Guid GetShaderGuid() {
			return mMaterial->GetShader().guid;
		}
		RscHandle<Material> GetMaterial() {
			return mMaterial;
		}
		parray<Vertex>& GetVertices() {
			return mVertices;
		}
		vector<uint32_t>& GetIndices() {
			return mIndices;
		}
	};
};
//#include "mesh_gen.inl"
namespace engineapi {
	template<typename T>
		requires std::is_base_of_v<Vertex, T>
	inline Mesh::Mesh(vector<T>& vertices, vector<uint32_t>& indices)
		: Asset(&refl::TypeInfo<Mesh>::StaticClass), mVertices(vertices), mIndices(indices)
	{
		auto cls = &refl::TypeInfo<Mesh>::StaticClass;
	}
}
