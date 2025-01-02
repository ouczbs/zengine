#pragma once
#include "asset/asset.h"
#include "render/render_type.h"
namespace api {
	class Texture : public Resource<Texture>
	{
	public:
		TextureDesc	mDesc;
	public:
		Texture(TextureDesc& desc);
	};
}