#include <NoesisPCH.h>

#define CBUFFER_SIZE 16 * 1024
namespace api{
    class UIRenderDevice final : public Noesis::RenderDevice, public UIRenderBuffer {
    public:
        RenderAPI*          mApi;
        UIRenderSystem*     mRenderSystem;
        Noesis::DeviceCaps  mCaps;
        std::vector<TextureBarrier>       mTextureBarriers;
        bool mStereoSupport;
        SINGLETON_IMPL(UIRenderDevice)
    public:
        UIRenderDevice(UIRenderSystem* system) : mApi(RenderAPI::Ptr()) , mRenderSystem(system){
            SINGLETON_PTR();
            ResourceMemoryUsage memoryUsage = ResourceMemoryUsage::HOST_VISIBLE |
                ResourceMemoryUsage::HOST_COHERENT | ResourceMemoryUsage::GPU_ONLY;
            mVertices.InitBuffer(DYNAMIC_VB_SIZE, BufferUsage::VERTEX, memoryUsage);
            mIndices.InitBuffer(DYNAMIC_IB_SIZE, BufferUsage::INDEX, memoryUsage);
            mConstants[0].InitBuffer(CBUFFER_SIZE, BufferUsage::UNIFORM, memoryUsage);
            mConstants[1].InitBuffer(CBUFFER_SIZE, BufferUsage::UNIFORM, memoryUsage);
            mConstants[2].InitBuffer(CBUFFER_SIZE, BufferUsage::UNIFORM, memoryUsage);
            mConstants[3].InitBuffer(CBUFFER_SIZE, BufferUsage::UNIFORM, memoryUsage);
        }
        ~UIRenderDevice();
        RenderContext& Context() {
            return mApi->context;
        }
        bool IsValidShader(Noesis::Shader shader, Noesis::RenderState state, bool hasExtendedDynamicState);
    private:
        /// From RenderDevice
        //@{
        const Noesis::DeviceCaps& GetCaps() const override;
        Noesis::Ptr<Noesis::RenderTarget> CreateRenderTarget(const char* label, uint32_t width,
            uint32_t height, uint32_t sampleCount, bool needsStencil) override;
        Noesis::Ptr<Noesis::RenderTarget> CloneRenderTarget(const char* label,
            Noesis::RenderTarget* surface) override;
        Noesis::Ptr<Noesis::Texture> CreateTexture(const char* label, uint32_t width, uint32_t height,
            uint32_t numLevels, Noesis::TextureFormat::Enum format, const void** data) override;
        void UpdateTexture(Noesis::Texture* texture, uint32_t level, uint32_t x, uint32_t y,
            uint32_t width, uint32_t height, const void* data) override;
        void BeginOffscreenRender() override;
        void EndOffscreenRender() override;
        void BeginOnscreenRender() override;
        void EndOnscreenRender() override;
        void SetRenderTarget(Noesis::RenderTarget* surface) override;
        void BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile) override;
        void EndTile(Noesis::RenderTarget* surface) override;
        void ResolveRenderTarget(Noesis::RenderTarget* surface, const Noesis::Tile* tiles,
            uint32_t numTiles) override;
        void* MapVertices(uint32_t bytes) override;
        void UnmapVertices() override;
        void* MapIndices(uint32_t bytes) override;
        void UnmapIndices() override;
        void DrawBatch(const Noesis::Batch& batch) override;
        //@}
    };
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static TinyImageFormat VKFormat(Noesis::TextureFormat::Enum format, bool sRGB)
	{
		switch (format)
		{
		case Noesis::TextureFormat::RGBA8: return sRGB ? TinyImageFormat_B8G8R8A8_SRGB : TinyImageFormat_B8G8R8A8_UNORM;
		case Noesis::TextureFormat::RGBX8: return sRGB ? TinyImageFormat_B8G8R8A8_SRGB : TinyImageFormat_B8G8R8A8_UNORM;
		case Noesis::TextureFormat::R8: return TinyImageFormat_R8_UNORM;
		default: NS_ASSERT_UNREACHABLE;
		}
	}
}