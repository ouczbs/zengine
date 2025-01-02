#include "os/package_path.h"
#include "os/file_manager.h"
namespace api {
	pmr::string PackagePath::RealPath() const
	{
		return FileManager::Ptr()->RealPath(*this);
	}
	pmr::string PackagePath::RealPath(const char* str)
	{
		return FileManager::Ptr()->RealPath(str);
	}
}

