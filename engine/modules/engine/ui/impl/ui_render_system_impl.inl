#include "ui/ui_render_system.h"
#include "ui/ui_module.h"
#include "ui_render_device.inl"

namespace api {
	SINGLETON_DEFINE(UIRenderDevice)
	SINGLETON_DEFINE(UIRenderSystem)
	UIRenderSystem::UIRenderSystem()
	{
		SINGLETON_PTR();
	}
	void UIRenderSystem::Initialize()
	{
		UIRenderDevice* impl = new UIRenderDevice(this);
		mRenderBuffer = impl;
		InitNoesisRender(impl->mCaps.linearRendering, impl->mStereoSupport, SAMPLE_COUNT_1);
	}
	void UIRenderSystem::Finalize()
	{

	}
	Noesis::RenderDevice* UIRenderSystem::GetRenderDevice()
	{
		return UIRenderDevice::Ptr();
	}
	inline bool UIRenderSystem::IsValidShader(Noesis::Shader shader, Noesis::RenderState state, bool hasExtendedDynamicState)
	{
		return UIRenderDevice::Ptr()->IsValidShader(shader, state, hasExtendedDynamicState);
	}
	UIRenderDevice::~UIRenderDevice()
	{
		int b = 10000;
		for (int a = 1; a < b; a++) {
			b--;
		}
	}
	bool UIRenderDevice::IsValidShader(Noesis::Shader shader, Noesis::RenderState state, bool hasExtendedDynamicState)
	{
		if (!hasExtendedDynamicState)
		{
			if (!IsValidStencilMode(shader, (Noesis::StencilMode::Enum)state.f.stencilMode)) return false;
		}
		else
		{
			if (state.f.stencilMode != 0) return false;
		}

		if (!IsValidBlendMode(shader, (Noesis::BlendMode::Enum)state.f.blendMode)) return false;
		if (!IsValidColorEnable(shader, state.f.colorEnable > 0)) return false;
		if (!IsValidWireframe(shader, state.f.blendMode > 0)) return false;
		return true;
	}
	const Noesis::DeviceCaps& UIRenderDevice::GetCaps() const
	{
		return mCaps;
	}
	Noesis::Ptr<Noesis::RenderTarget> UIRenderDevice::CreateRenderTarget(const char* label, uint32_t width, uint32_t height, uint32_t sampleCount, bool needsStencil)
	{
		return Noesis::Ptr<Noesis::RenderTarget>();
	}
	Noesis::Ptr<Noesis::RenderTarget> UIRenderDevice::CloneRenderTarget(const char* label, Noesis::RenderTarget* surface)
	{
		return Noesis::Ptr<Noesis::RenderTarget>();
	}
	Noesis::Ptr<Noesis::Texture> UIRenderDevice::CreateTexture(const char* label, uint32_t width, uint32_t height, uint32_t numLevels, Noesis::TextureFormat::Enum format, const void** data)
	{
		TextureDesc desc{};
		desc.id = -1;
		desc.format = VKFormat(format, mCaps.linearRendering);
		desc.width = width;
		desc.height = height;
		desc.mipLevel = numLevels;
		desc.sampleCount = SAMPLE_COUNT_1;
		desc.usage = TextureUsage::SAMPLEABLE | TextureUsage::BLIT_DST;
		desc.state = ResourceState::UNDEFINED;
		desc.dimension = TextureDimension::TEX_2D;
		desc.depth = 1;
		desc.arraySize = 1;
		mApi->CreateTexture(desc);
		return Noesis::MakePtr<UITexture>(desc);
	}
	void UIRenderDevice::UpdateTexture(Noesis::Texture* texture, uint32_t level, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
	{
		UITexture* uiTex = (UITexture*)texture;
		TextureUpdateArgs args{.x = x, .y = y, .width = width,.height = height, .mipLevel = level, .data = data};
		Context().UpdateTexture(uiTex->desc, args, ResourceState::TRANSFER_DST);
		for (auto& barrier : mTextureBarriers) {
			if (barrier.mTexture.image == uiTex->desc.image) {
				return;
			}
		}
		mTextureBarriers.push_back(uiTex->desc.ToBarrier(ResourceState::TRANSFER_DST, ResourceState::READ_ONLY));
	}
	void UIRenderDevice::BeginOffscreenRender()
	{
		int b = 10000;
		for (int a = 1; a < b; a++) {
			b--;
		}
	}
	void UIRenderDevice::EndOffscreenRender()
	{
		if (!mTextureBarriers.empty()) {
			ResourceBarrierDesc desc{};
			desc.textureBarriersCount = mTextureBarriers.size();
			desc.pTextureBarriers = mTextureBarriers.data();
			Context().ExecuteResourceBarriers(desc);
			mTextureBarriers.clear();
		}
	}
	void UIRenderDevice::BeginOnscreenRender()
	{
		mRenderSystem->BeginRender(&mApi->context);
	}
	void UIRenderDevice::EndOnscreenRender()
	{
		int b = 10000;
		for (int a = 1; a < b; a++) {
			b--;
		}
	}
	void UIRenderDevice::SetRenderTarget(Noesis::RenderTarget* surface)
	{
		int b = 10000;
		for (int a = 1; a < b; a++) {
			b--;
		}
	}
	void UIRenderDevice::BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile)
	{
		int b = 10000;
		for (int a = 1; a < b; a++) {
			b--;
		}
	}
	void UIRenderDevice::EndTile(Noesis::RenderTarget* surface)
	{
		int b = 10000;
		for (int a = 1; a < b; a++) {
			b--;
		}
	}
	void UIRenderDevice::ResolveRenderTarget(Noesis::RenderTarget* surface, const Noesis::Tile* tiles, uint32_t numTiles)
	{
		int b = 10000;
		for (int a = 1; a < b; a++) {
			b--;
		}
	}
	void* UIRenderDevice::MapVertices(uint32_t bytes)
	{
		auto& ctx = Context();
		return mVertices.MapBuffer(bytes, ctx.frameNumber, ctx.frameNumber - ctx.frameCount);
	}
	void UIRenderDevice::UnmapVertices()
	{
	}
	void* UIRenderDevice::MapIndices(uint32_t bytes)
	{
		auto& ctx = Context();
		return mIndices.MapBuffer(bytes, ctx.frameNumber, ctx.frameNumber - ctx.frameCount);
	}
	void UIRenderDevice::UnmapIndices()
	{
	}
	void UIRenderDevice::DrawBatch(const Noesis::Batch& batch)
	{
		EndOffscreenRender();
		mRenderSystem->DrawBatch(batch);
	}
}
