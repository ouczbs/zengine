#include "scene/scene.h"
#include "scene/level_blueprint.h"
namespace api {
	Scene::Scene()
	{

	}
	Scene::~Scene()
	{
	}
	void Scene::OnLoad()
	{
		if (mLevelBlueprint) {
			mLevelBlueprint->LoadScene(this);
		}
	}
	void Scene::Update()
	{
	}
	void Scene::Render()
	{
	}
	void Scene::AddLevel(Level* level)
	{
		mLevels.push_back(level);
	}
	void Scene::AddGameObject(GameObject* object)
	{
		mObjects.push_back(object);
	}
}
