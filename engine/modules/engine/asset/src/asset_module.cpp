#include "asset/asset_module.h"
#include "asset/resource_system.h"
#include "asset/asset_loader.h"
#include "archive/pch.h"
namespace api {
	void AssetModule::OnLoad(int argc, char** argv)
	{
		InitResourceType();
		AddSystem<ResourceSystem>();
		AssetLoader::Init();
		TextArchive::Register<Guid>();
	}
	void AssetModule::OnUnload()
	{

	}
}