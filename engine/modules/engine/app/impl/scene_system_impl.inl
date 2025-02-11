#include "scene/scene_system.h"
#include "scene/scene.h"
namespace api {
	class SceneSystemImpl
	{
	public:
		Scene* curScene = nullptr;
		table<Name, Scene*> scenes;
		table<Name, Name>   pathTable;
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
		Scene* FindScene(PackagePath path);
	};
	Scene* SceneSystemImpl::GetScene(Name name)
	{
		auto iter = scenes.find(name);
		if (iter != scenes.end())
			return iter->second;
		return nullptr;
	}
	Scene* SceneSystemImpl::FindScene(PackagePath path)
	{
		Name name = Name::Find(path.path);
		auto it = pathTable.find(name);
		if (it != pathTable.end()) {
			return GetScene(it->second);
		}
		return nullptr;
	}
	void SceneSystemImpl::LoadScene(PackagePath path, bool switchNow)
	{
		Scene* pScene = FindScene(path);
		if (pScene) {
			curScene = pScene;
			return;
		}
		FileHandle handle(path);
		if (!handle.Open(FILE_OP::READ)) {
			return;
		}
		pmr::string text = handle.ReadAll<pmr::string>();
		pScene = new Scene();
		if (!TextDeserialize<Scene>(text, pScene)) _UNLIKELY{
			delete pScene;
			return;
		};
		pScene->mPath = path();
		pathTable.emplace(pScene->mPath, pScene->mName);
		scenes.emplace(pScene->mName, pScene);
		curScene = pScene;
		curScene->OnLoad();
	}
	void SceneSystemImpl::SwitchScene(Name name)
	{
		auto scene = GetScene(name);
		if (scene == nullptr) _UNLIKELY
		{
			zlog::error("Switch to invalid scene: {}", name.data());
			return;
		}
		curScene = scene;
	}
	void SceneSystemImpl::DeleteScene(Name name)
	{
		auto iter = scenes.find(name);
		if (iter == scenes.end()) _UNLIKELY
		{
			zlog::info("Attempt to delete a nonexistent scene: {}", name.ToStringView());
			return;
		}
		delete iter->second;
		scenes.erase(iter);
	}
	void SceneSystemImpl::DeleteAllScene()
	{
		for (auto& iter : scenes)
		{
			delete iter.second;
		}
		scenes.clear();
	}
	void SceneSystemImpl::ReloadScene()
	{
		if (!curScene)_UNLIKELY{
			return;
		}
		Name path = curScene->mPath;
		DeleteScene(curScene->mName);
		LoadScene(path, true);
	}
	void SceneSystemImpl::Render()
	{
		if(curScene) _LIKELY
			curScene->Render();
	}
	void SceneSystemImpl::Update()
	{
		if(curScene) _LIKELY
			curScene->Update();
	}
	Scene* SceneSystemImpl::GetCurScene()
	{
		return curScene;
	}
	SINGLETON_DEFINE(SceneSystem)
	SceneSystem::SceneSystem()
	{
		SINGLETON_PTR();
		impl = new (GlobalPool())SceneSystemImpl();
	}
	void SceneSystem::Initialize()
	{

	}
	void SceneSystem::Finalize()
	{

	}
	void SceneSystem::LoadScene(PackagePath path, bool switchNow)
	{
		impl->LoadScene(path, switchNow);
	}
	void SceneSystem::SwitchScene(Name name)
	{
		impl->SwitchScene(name);
	}
	void SceneSystem::DeleteScene(Name name)
	{
		impl->DeleteScene(name);
	}
	void SceneSystem::DeleteAllScene()
	{
		impl->DeleteAllScene();
	}
	void SceneSystem::ReloadScene()
	{
		impl->ReloadScene();
	}
	void SceneSystem::Render()
	{
		impl->Render();
	}
	void SceneSystem::Update()
	{
		impl->Update();
	}
	Scene* SceneSystem::GetCurScene()
	{
		return impl->GetCurScene();
	}
	Scene* SceneSystem::GetScene(Name name)
	{
		return impl->GetScene(name);
	}
}