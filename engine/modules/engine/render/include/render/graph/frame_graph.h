#pragma once
#include "frame_graph_node.h"
namespace api {
	struct FRenderView;
	using pmr::Tag;
	class FrameGraph
	{
	public:
		struct TextureID{
			int32_t  id;
			uint32_t tick;
		};
		FrameGraphNodePtr							mCurrentNode;
		uint32_t									mTickStamp{1};
		uint32_t									mSurfaceID{0};
		table<TextureSampler, void*>				mTextureSamplerPool;
		table<Tag, uint32_t>						mTextureTagMap;
		table<TextureKey, std::vector<TextureID>>	mTextureKeyMap;//一张图不够用了
		pmr::vector<TextureDesc>					mTexturePool{GlobalPool()};
		table<TextureViewKey, ImageViewPtr>			mTextureViewPool;
		pmr::vector<FrameGraphNodePtr>				mNodes{FramePool()};
		RenderPassNode* mFirstInputNode{ nullptr };
		RenderPassNode* mLastOutputNode{ nullptr };
		inline static Name		 NameSurface{ "surface" };
#ifdef WITH_EDITOR
		inline static Name		 NameEditorSurface{ "editor_surface" };
		bool								 mIsRenderEditorSurface{false};
		uint32_t							 mEditorSurfaceID{ 0 };
		TextureDesc& GetEditorSurface() { return ResolveTexture(mEditorSurfaceID); }
#endif // 
	public:
		FrameGraphNodePtr GetCurrentNode() { return mCurrentNode; };
		template<typename T>
		FrameGraphNodePtr AddRenderPass() { return AddRenderPass(&T::Setup, &T::Execute); }
		FrameGraphNodePtr AddRenderPass(const RenderPassSetupFunction& setup, const RenderPassNodeExecuteFn& executor);
		RenderPassBuilder CreateRenderPassBuild();
		void InitSurface(TextureDesc* surfaces, uint32_t frames);
		void Input(const TextureDesc& surfaces) { mSurfaceID = surfaces.id; };
		void Setup();
		void Compile();
		void Execute(FRenderView& view);
		void Clear();
		void CullGraph();
		void FillGraph();
		TextureDesc&  GetSurface() { return ResolveTexture(mSurfaceID); }
		TextureDesc&  GetRenderSurface();
		void		  TransitionState(TextureDesc& desc, ResourceState state);
		bool		  ResolveState(TextureDesc& desc, ResourceState& srcstart, ResourceState& dststate);
		bool		  ResolveState(BufferDesc& desc, ResourceState& srcstart, ResourceState& dststate);
		TextureDesc&  ResolveTexture(int32_t id);
		TextureDesc   ResourceTexture(Name name, int num);
		ImageViewPtr  ResolveTextureView(TextureDesc& desc);
		ImageViewPtr  ResolveTextureView(TextureViewKey key);
		void*		  ResolveTextureSampler(TextureSampler sampler);
		void		  SetResourceTexture(TextureDesc texture, Name name, int num = 0);
		uint32_t	  GetTextureID(Name name, int num);
		void		  AcquireTexture(TextureDesc& desc);
		void		  RealeaseTexture(TextureDesc& desc);
	public:
		void ExecutePresentPass(FRenderView& view);
		void ExecuteRenderPass(RenderPassNode* node, FRenderView& view);
		void ExecuteComputePass(RenderPassNode* node, FRenderView& view);
		void ExecuteCopyPass(RenderPassNode* node, FRenderView& view);

		static void ExecuteResourceBarriers(RenderPassNode* node, RenderPassType type);
	};
}