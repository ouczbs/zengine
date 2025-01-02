#pragma once
#include "asset/asset.h"
#include "render/type.h"
namespace api {
	class Shader;
	class ShaderProgram : public Resource<ShaderProgram> {
	protected:
		GENERATED_BODY()
		UPROPERTY()
		RscHandle<Shader> mOwner;
		UPROPERTY()
		ShaderStage mStage;
	public:
		refl::Any Meta() {
			return refl::Any{this};
		}
		ShaderProgram() : mOwner{},mStage(ShaderStage::NONE) {}
		ShaderProgram(ShaderStage stage) : mStage(stage) {};
		ShaderStage GetStage() {
			return mStage;
		}
		RscHandle<Shader> GetOwner() {
			return mOwner;
		}
		void SetOwner(RscHandle<Shader> owner) {
			mOwner = owner;
		}
		pmr::Name ubName();
	};
	class Shader : public Asset {
	private:
		GENERATED_BODY()
		pmr::Name mUBName;
		UPROPERTY()
		RscHandle<ShaderProgram> mVert;
		UPROPERTY()
		RscHandle<ShaderProgram> mFrag;
		friend class Scene;
	public:
		Shader();
		~Shader();
		refl::Any Meta() {
			return refl::Any{ this };
		}
		void ubName(pmr::Name name) {
			mUBName = name;
		}
		pmr::Name ubName() {
			return mUBName;
		}
		void BeginLoad();
		template<typename T = ShaderProgram>
		RscHandle<T> GetVertHandle() {
			return mVert;
		}
		template<typename T = ShaderProgram>
		RscHandle<T> GetFragHandle() {
			return mFrag;
		}
		void SetVertHandle(RscHandle<ShaderProgram> vert) {
			mVert = vert;
			mVert->SetOwner(GetHandle());
		}
		void SetFragHandle(RscHandle<ShaderProgram> frag) {
			mFrag = frag;
			mFrag->SetOwner(GetHandle());
		}
	};
	inline pmr::Name api::ShaderProgram::ubName()
	{
		return mOwner ? mOwner->ubName() : "";
	}
};
#include ".render/shader_gen.inl"