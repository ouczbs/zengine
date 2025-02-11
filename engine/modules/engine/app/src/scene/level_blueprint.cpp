#include "scene/level_blueprint.h"
#include "scene/scene.h"
#include "os/file_handle.h"
#include "archive/pch.h"
namespace api {
	LevelBlueprint::LevelBlueprint()
	{
	}
	LevelBlueprint::~LevelBlueprint()
	{
	}
	void LevelBlueprint::LoadScene(Scene* scene)
	{
		for (auto& levelInfo : scene->mLevelInfos) {
			FileHandle handle(levelInfo.mPath);
			if (!handle.Open(FILE_OP::READ)) {
				continue;
			}
			Level* pLevel = new Level(levelInfo);
			pmr::string text = handle.ReadAll<pmr::string>();
			if (!TextDeserialize<Level>(text, pLevel)) {
				delete pLevel;
				continue;
			}
			scene->AddLevel(pLevel);
		}
	}
}
