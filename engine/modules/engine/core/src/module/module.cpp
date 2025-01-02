#include "module/module_manager.h"
#include "os/file_manager.h"
#include "archive/pch.h"
namespace api {
	void CoreModule::OnLoad(int argc, char** argv)
	{
		TextArchive::InitSerde();
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
