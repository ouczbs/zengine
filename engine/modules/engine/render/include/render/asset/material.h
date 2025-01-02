#pragma once
#include "shader.h"
namespace api {
	class Material : public Asset {
	protected:
		RscHandle<Shader> mShader;

		friend class Scene;
	public:
		Material(RscHandle<Shader> shader) : Material() { mShader = shader; };
		Material();
		~Material();
		void BeginLoad() {
			mShader->BeginLoad();
		}
		RscHandle<Shader> GetShader() {
			return mShader;
		}
		void SetShader(RscHandle<Shader> shader) {
			mShader = shader;
		}
	};
	class MaterialInstance {
		MaterialInfo mInfo{};
		RscHandle<Material> mMaterial;
	public:
		MaterialInstance() {}
		MaterialInfo& GetInfo() {
			return mInfo;
		}
		void SetInfo(const MaterialInfo& info) {
			mInfo = info;
		}
		RscHandle<Material> GetMaterial() {
			return mMaterial;
		}
		void SetMaterial(const RscHandle<Material>& material) {
			mMaterial = material;
		}
		Guid GetShaderGuid() {
			return mMaterial->GetShader().guid;
		}
		Material* operator->() {
			return mMaterial.Ptr();
		}
		template<typename T>
		bool SetResource(MaterialResource* pResource, const T& t) {
			if (!pResource) {
				return false;
			}
			auto obj = pResource->obj;
#ifdef API_DEBUG
			if (!obj.Check(meta_info<T>())) {
				throw std::runtime_error("Error: SetResource with error class type");
			}
#endif // API_DEBUG
			*(obj.CastTo<T*>()) = t;
			pResource->isDirty = true;
			return true;
		}
		template<typename T>
		bool SetStaticResource(Name name,const T& t) {
			auto pResource = mInfo.staticBlock.FindMaterialResource(name);
			return SetResource(pResource, t);
		}
		template<typename T>
		bool SetClassResource(Name name, const T& t) {
			auto pResource = mInfo.classBlock.FindMaterialResource(name);
			return SetResource(pResource, t);
		}
	};
};