#include "editor/editor_system.h"
namespace api {
	SINGLETON_DEFINE(EditorSystem)
	EditorSystem::EditorSystem()
	{
		SINGLETON_PTR();
	}
	ImTextureID EditorSystem::AddTexture(FrameGraph& graph, TextureDesc& desc, TextureSampler key)
	{
		ImageViewPtr imageview = graph.ResolveTextureView(desc);
		SamplerPtr sampler = graph.ResolveTextureSampler(key);
		return AddTexture(imageview, sampler, desc.state);
	}
}