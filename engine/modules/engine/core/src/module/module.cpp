#include "module/module_manager.h"
#include "os/file_manager.h"
#include "archive/pch.h"
#include "archive/reflect.h"
namespace api {
	void CoreModule::OnLoad(int argc, char** argv)
	{
		TextArchive::InitSerde();
		TextArchive::Register<Vector2>();
		TextArchive::Register<Vector3>();
		TextArchive::Register<Vector4>();
	}
	void CoreModule::OnUnload()
	{

	}
	void CoreModule::InitMetaData(void)
	{
		
	}
	IModule::~IModule()
	{
		for (auto system : mSystems) {
			delete system;
		}
	}
	void IModule::Initialize()
	{
		for (auto system : mSystems) {
			system->Initialize();
		}
	}
	void IModule::Finalize()
	{
		for (auto system : mSystems) {
			system->Finalize();
		}
	}
}
