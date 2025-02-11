#pragma once
#include "level.h"
namespace api {
	class LevelBlueprint;
	struct SceneInfo {
		UPROPERTY()
		Name					mName;
		Name					mPath;
		UPROPERTY()
		TAny<LevelBlueprint>	mLevelBlueprint;
		UPROPERTY()
		vector<LevelInfo>		mLevelInfos;
	};
	class Scene : public SceneInfo{
	protected:
		GENERATED_BODY()

		vector<Level*>      mLevels;
		vector<GameObject*> mObjects;
	public:
		Scene(const SceneInfo& info) : SceneInfo(info) {};
		Scene();
		~Scene();
		void OnLoad();
		void Update();
		void Render();
		void AddLevel(Level* level);
		void AddGameObject(GameObject* object);
	};
}
#include ".app/scene_gen.inl"