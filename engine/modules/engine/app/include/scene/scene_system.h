#pragma once
#include "module/module.h"
namespace api {
	class Scene;
	class SceneSystemImpl;
	class APP_API SceneSystem : public ISystem {
		SINGLETON_IMPL(SceneSystem)
	private:
		SceneSystemImpl* impl;
	public:
		SceneSystem();
		void Initialize() override;
		void Finalize() override;
	public:
		void LoadScene(PackagePath path, bool switchNow = true);
		void SwitchScene(Name name);
		void DeleteScene(Name name);
		void DeleteAllScene();
		void ReloadScene();
		void Render();
		void Update();
		Scene* GetCurScene();
		Scene* GetScene(Name name);
	};
}