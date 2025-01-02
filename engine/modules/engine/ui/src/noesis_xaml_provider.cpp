#include "ui/noesis_xaml_provider.h"
#include "os/package_path.h"
#include <NsGui/Uri.h>
#include <NsGui/Stream.h>
namespace api {
	using namespace Noesis;
	NoesisXamlProvider::NoesisXamlProvider()
	{

	}
	NoesisXamlProvider::~NoesisXamlProvider()
	{
	}
	Ptr<Stream> NoesisXamlProvider::LoadXaml(const Uri& uri)
	{
		return OpenFileStream(PackagePath::RealPath(uri.FullPath()).c_str());
	}
}
