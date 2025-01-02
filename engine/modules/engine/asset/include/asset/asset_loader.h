#pragma once
#include "asset.h"
#include "os/package_path.h"
namespace api {
	class AssetLoader :public IFileLoader{
	public:
		static void Init();
		ResourceBundle LoadFile(PackagePath path, const MetaBundle& meta) override;
		void SaveFile(PackagePath path, const ResourceBundle& bundle) override;
	};
}