#include "ui/noesis_font_provider.h"
#include <NsGui/Uri.h>
#include <NsGui/Stream.h>
namespace api {
	using namespace Noesis;
	NoesisFontProvider::NoesisFontProvider()
	{

	}
	NoesisFontProvider::~NoesisFontProvider()
	{
	}
	FontSource NoesisFontProvider::MatchFont(const Uri& baseUri, const char* familyName, FontWeight& weight, FontStretch& stretch, FontStyle& style)
	{
		auto uri = baseUri;
		auto a1 = uri.Str();
		auto a2 = uri.FullPath();
		return FontSource();
	}
	bool NoesisFontProvider::FamilyExists(const Uri& baseUri, const char* familyName)
	{
		auto uri = baseUri;
		auto a1 = uri.Str();
		auto a2 = uri.FullPath();
		return false;
	}
}