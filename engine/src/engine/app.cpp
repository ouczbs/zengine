#include "engine/app.h"
#include "scene/scene_system.h"
#include "os/file_manager.h"

#include "scene/scene.h"
#include "scene/level_blueprint.h"
#include "object/component/transform.h"
#include "os/file_handle.h"
#include "archive/pch.h"
namespace api{
	void CreateDemoScene() {
		LevelInfo levelInfo{ "main", "/scene/entry/main.level", Vector3(0, 255, 0) };
		SceneInfo sceneInfo{};
		sceneInfo.mName = "entry";
		sceneInfo.mPath = "/scene/entry.scene";
		sceneInfo.mLevelBlueprint = TAny<LevelBlueprint>(new LevelBlueprint());
		sceneInfo.mLevelInfos.push_back(levelInfo);

		Level level{ levelInfo };
		GameObject* obj1 = new GameObject();
		level.AddObject(obj1);
		{
			FileHandle handle(levelInfo.mPath);
			handle.Open(FILE_OP::WRITE);
			handle.Write(TextSerialize(level));
		}
		Scene scene{ sceneInfo };
		scene.AddLevel(&level);
		{
			FileHandle handle(sceneInfo.mPath);
			handle.Open(FILE_OP::WRITE);
			handle.Write(TextSerialize(scene));
		}
	}
    bool App::Launch()
	{
		PackagePath scenePath{ "/engine/assets/scene" };
		FileManager::Ptr()->Mount("scene", scenePath.RealPath().c_str());
		CreateDemoScene();
		gProjectConfig.EntryScene = "/scene/entry.scene";
		SceneSystem::Ptr()->LoadScene(gProjectConfig.EntryScene);
		return true;
	}
	void App::Update()
	{
#ifdef WITH_EDITOR
		gEditorConfig.IsGameStart = true;
		if(gEditorConfig.IsGameStart && !gEditorConfig.isGamePause)
			SceneSystem::Ptr()->Update();
#else 
		SceneSystem::Ptr()->Update();
#endif // WITH_EDITOR
	}
}