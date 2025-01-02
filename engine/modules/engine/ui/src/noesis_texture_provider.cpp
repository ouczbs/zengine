#include "ui/noesis_texture_provider.h"
#include "os/package_path.h"
#include <NsGui/Uri.h>
#include <NsGui/Stream.h>
#include <NsCore/Log.h>
#include <NsRender/Texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace api {
    using namespace Noesis;
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static int Read(void* user, char* data, int size)
    {
        Stream* stream = (Stream*)user;
        return stream->Read(data, size);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static void Skip(void* user, int n)
    {
        Stream* stream = (Stream*)user;
        stream->SetPosition((int)stream->GetPosition() + n);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    static int Eof(void* user)
    {
        Stream* stream = (Stream*)user;
        return stream->GetPosition() >= stream->GetLength();
    }
	NoesisTextureProvider::NoesisTextureProvider()
	{

	}
	NoesisTextureProvider::~NoesisTextureProvider()
	{
	}
	TextureInfo NoesisTextureProvider::GetTextureInfo(const Uri& uri)
	{
        TextureInfo info;
        Ptr<Stream> file = OpenFileStream(PackagePath::RealPath(uri.FullPath()).c_str());
        if (file != 0)
        {
            int x, y, n;
            stbi_io_callbacks callbacks = { Read, Skip, Eof };
            if (stbi_info_from_callbacks(&callbacks, file, &x, &y, &n))
            {
                info.width = x;
                info.height = y;
            }
            else
            {
                NS_LOG_WARNING("%s: %s", uri.Str(), stbi_failure_reason());
            }
            file->Close();
        }
        return info;
	}
	Ptr<Texture> NoesisTextureProvider::LoadTexture(const Uri& uri, RenderDevice* device)
	{
		auto a1 = uri.Str();
		auto a2 = uri.FullPath();
		return Ptr<Texture>();
	}
}