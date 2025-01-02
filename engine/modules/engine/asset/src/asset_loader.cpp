#include "asset/asset_loader.h"
#include "os/file_manager.h"
#include "os/file_handle.h"
#include <fstream>
namespace api {
	void AssetLoader::Init() {
		ResourceSystem::Ptr()->RegisterLoader<AssetLoader>(".asset");
	}
	ResourceBundle AssetLoader::LoadFile(PackagePath path, const MetaBundle& metas)
	{
		ResourceBundle bundle;
		return bundle;
	}
	void AssetLoader::SaveFile(PackagePath path, const ResourceBundle& bundle)
	{
		MetaBundle new_meta = ResourceSystem::Ptr()->GetVisitMeta(bundle);
		ResourceSystem::Ptr()->SaveMeta(path, new_meta);
	}
}
