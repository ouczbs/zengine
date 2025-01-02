#pragma once
#include "engine_config.h"
#include "project_config.h"
namespace api {
	constexpr const char* CFileResourcePath = "/work/assets/file_resource.meta";
	constexpr const char* CFileFlagName = "/work/assets/file_flag.meta";
	constexpr const char* CFileMapName = "/work/assets/file_map.meta";
	constexpr const char* CFileMountName = "/work/assets/file_mount.meta";

	extern APP_API EngineConfig gEngineConfig;
}