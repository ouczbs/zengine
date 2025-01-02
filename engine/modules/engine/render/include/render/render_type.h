#pragma once
#include "tinyimageformat/tinyimageformat_base.h"
#include "math/vector2.h"
#include "math/vector4.h"
#include "asset/asset.h"
#include <functional>
namespace api {
	enum class RenderPassType : uint8_t {
		Render,
		Present,
		Compute,
		Copy
	};
	enum class AttachmentFlag : uint8_t {
		None = 0,
		Clear = 0x01,
		DiscardStart = 0x02,
		DiscardEnd = 0x04,
		DepthStencil = 0x08,
		MainPass = 0x10,
		Subpass = 0x20,
		Sample = 0x40,
	};
	enum class RenderPassNodeType : uint8_t {
		None,
		Scene,
		Imgui,
	};
	enum class RenderPassNodeFlag : uint8_t
	{
		None = 0,
		Output = 0x01,
		FirstInput = 0x02,
		LastOutput = 0x04,
        WaitTransfer = 0x08,
	};
    enum class BufferUsage : uint8_t {
        None = 0,
        STATIC,      //!< content modified once, used many times
        DYNAMIC,     //!< content modified frequently, used many times
        VERTEX,
        INDEX,
        UNIFORM,
        SHADER_STORAGE,
        TRANSFER_SRC,
    };
    /*
    扩展的内存用途标志：
    GPU_RENDERING：专门为渲染操作设计的内存，常用于存储纹理、帧缓冲等图形资源。
    GPU_STORAGE：用于存储不需要频繁更新的数据，例如大容量缓冲区或其他不常变动的资源。
    GPU_COMPUTE：用于计算着色器或其他 GPU 计算任务。可以是读取和写入的内存类型，但它主要用于计算而非渲染。

    与主机的交互：
    HOST_VISIBLE：标志该内存可以被主机访问，适用于从 CPU 到 GPU 或反向传输的场景。
    HOST_COHERENT：标志该内存在 CPU 和 GPU 之间的一致性，不需要显式同步。

    懒加载和静态数据：
    LAZY_ALLOCATED：懒加载内存，适用于 GPU 程序中的延迟加载资源。
    STATIC：静态数据，如常量缓冲区、着色器程序，通常在初始化时填充并且在后续使用中不变化。
    */
    enum class ResourceMemoryUsage : uint16_t
    {
        /// No intended memory usage specified.
        UNKNOWN = 0,
        /// Memory will be used on device only, no need to be mapped on host.
        GPU_ONLY = 0x01,
        /// Memory will be mapped on host. Could be used for transfer to device.
        CPU_ONLY = 0x02,
        /// Memory will be used for frequent (dynamic) updates from host and reads on device.
        CPU_TO_GPU = 0x04,
        /// Memory will be used for writing on device and readback on host.
        GPU_TO_CPU = 0x08,
        /// Memory is intended to be used for rendering or graphics purposes.
        GPU_RENDERING = 0x10,
        /// Memory will be used for storage or large buffers that don't require frequent updates.
        GPU_STORAGE = 0x20,
        /// Memory is visible to the host and the device, can be used for both read and write.
        HOST_VISIBLE = 0x40,
        /// Memory will be lazily allocated and can be used for GPU resources that are not immediately needed.
        LAZY_ALLOCATED = 0x80,
        /// Memory is coherent for both host and device, meaning no explicit synchronization is needed.
        HOST_COHERENT = 0x100,
        /// Static data, such as constant buffers or shader resources, that will not change after initialization.
        STATIC = 0x200,
        /// Memory intended for use by a compute shader, can be mapped or un-mapped depending on use case.
        GPU_COMPUTE = 0x400,
    };
    enum SampleCount : uint8_t
    {
        SAMPLE_COUNT_1 = 1,
        SAMPLE_COUNT_2 = 2,
        SAMPLE_COUNT_4 = 4,
        SAMPLE_COUNT_8 = 8,
        SAMPLE_COUNT_16 = 16,
        SAMPLE_COUNT_COUNT = 5,
    };
    enum class ResourceState : uint8_t
    {
        // The initial layout after the creation of the VkImage. We use this to denote the state before
        // any transition.
        UNDEFINED,
        // Fragment/vertex shader accessible layout for reading and writing.
        READ_WRITE,
        // Fragment/vertex shader accessible layout for reading only.
        READ_ONLY,
        // For the source of a copy operation.
        TRANSFER_SRC,
        // For the destination of a copy operation.
        TRANSFER_DST,
        // For using a depth texture as an attachment.
        DEPTH_ATTACHMENT,
        // For using a depth texture both as an attachment and as a sampler.
        DEPTH_SAMPLER,
        // For swapchain images that will be presented.
        PRESENT,
        // For color attachments, but also used when the image is a sampler.
        // TODO: explore separate layout policies for attachment+sampling and just attachment.
        COLOR_ATTACHMENT,
    };
    enum class TextureUsage :uint16_t {
        NONE = 0x0000,
        COLOR_ATTACHMENT = 0x0001,            //!< Texture can be used as a color attachment
        DEPTH_ATTACHMENT = 0x0002,            //!< Texture can be used as a depth attachment
        STENCIL_ATTACHMENT = 0x0004,          //!< Texture can be used as a stencil attachment
        UPLOADABLE = 0x0008,                  //!< Data can be uploaded into this texture (default)
        SAMPLEABLE = 0x0010,                  //!< Texture can be sampled (default)
        SUBPASS_INPUT = 0x0020,               //!< Texture can be used as a subpass input
        BLIT_SRC = 0x0040,                    //!< Texture can be used the source of a blit()
        BLIT_DST = 0x0080,                    //!< Texture can be used the destination of a blit()
        PROTECTED = 0x0100,                   //!< Texture can be used for protected content
        DEFAULT = UPLOADABLE | SAMPLEABLE     //!< Default texture usage
    };
    enum class TextureDimension : uint8_t
    {
        TEX_NULL = 0,
        TEX_1D = 0x01,
        TEX_2D = 0x02,
        TEX_3D = 0x04,
        TEX_CUBE = 0x08,
    };
    //! Sampler Wrap mode
    enum class SamplerWrapMode : uint8_t {
        CLAMP_TO_EDGE,      //!< clamp-to-edge. The edge of the texture extends to infinity.
        REPEAT,             //!< repeat. The texture infinitely repeats in the wrap direction.
        MIRRORED_REPEAT,    //!< mirrored-repeat. The texture infinitely repeats and mirrors in the wrap direction.
    };

    //! Sampler minification filter
    enum class SamplerMinFilter : uint8_t {
        // don't change the enums values
        NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
        LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
        NEAREST_MIPMAP_NEAREST = 2, //!< Mip-mapping is activated. But no filtering occurs.
        LINEAR_MIPMAP_NEAREST = 3,  //!< Box filtering within a mip-map level.
        NEAREST_MIPMAP_LINEAR = 4,  //!< Mip-map levels are interpolated, but no other filtering occurs.
        LINEAR_MIPMAP_LINEAR = 5    //!< Both interpolated Mip-mapping and linear filtering are used.
    };

    //! Sampler magnification filter
    enum class SamplerMagFilter : uint8_t {
        // don't change the enums values
        NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
        LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
    };

    //! Sampler compare mode
    enum class SamplerCompareMode : uint8_t {
        // don't change the enums values
        NONE = 0,
        COMPARE_TO_TEXTURE = 1
    };

    //! comparison function for the depth / stencil sampler
    enum class SamplerCompareFunc : uint8_t {
        // don't change the enums values
        LE = 0,     //!< Less or equal
        GE,         //!< Greater or equal
        L,          //!< Strictly less than
        G,          //!< Strictly greater than
        E,          //!< Equal
        NE,         //!< Not equal
        A,          //!< Always. Depth / stencil testing is deactivated.
        N           //!< Never. The depth / stencil test always fails.
    };
    using SamplerPtr = void*;
    //! Sampler parameters
    struct TextureSampler { // NOLINT
        SamplerMagFilter filterMag : 1;    //!< magnification filter (NEAREST)
        SamplerMinFilter filterMin : 3;    //!< minification filter  (NEAREST)
        SamplerWrapMode wrapS : 2;    //!< s-coordinate wrap mode (CLAMP_TO_EDGE)
        SamplerWrapMode wrapT : 2;    //!< t-coordinate wrap mode (CLAMP_TO_EDGE)

        SamplerWrapMode wrapR : 2;    //!< r-coordinate wrap mode (CLAMP_TO_EDGE)
        uint8_t anisotropyLog2 : 3;    //!< anisotropy level (0)
        SamplerCompareMode compareMode : 1;    //!< sampler compare mode (NONE)
        uint8_t padding0 : 2;    //!< reserved. must be 0.

        SamplerCompareFunc compareFunc : 3;    //!< sampler comparison function (LE)
        uint8_t padding1 : 5;    //!< reserved. must be 0.
        uint8_t padding2 : 8;    //!< reserved. must be 0.
        struct EqualTo {
            bool operator()(TextureSampler lhs, TextureSampler rhs) const noexcept {
                auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
                auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
                return *pLhs == *pRhs;
            }
        };

        struct LessThan {
            bool operator()(TextureSampler lhs, TextureSampler rhs) const noexcept {
                auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
                auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
                return *pLhs == *pRhs;
            }
        };

    private:
        friend inline bool operator < (TextureSampler lhs, TextureSampler rhs) noexcept {
            return TextureSampler::LessThan{}(lhs, rhs);
        }
    public:
        friend inline bool operator== (TextureSampler lhs, TextureSampler rhs) noexcept {
            return TextureSampler::EqualTo{}(lhs, rhs);
        }
    };
    using ImagePtr = void*;
    using ImageViewPtr = void*;
    using BufferPtr = void*;
    struct BufferBarrier;
    struct TextureBarrier;
    struct BufferDesc {
        BufferPtr           buffer;
        void*               pAllocation;
        void*               pMappingAddr;
        uint32_t            size;
        BufferUsage         usage;
        ResourceMemoryUsage memoryUsage;
        static BufferDesc Make() {
            return {};
        }
        BufferBarrier ToBarrier(ResourceState from, ResourceState to)const;
    };
    struct TextureViewKey {
        ImagePtr         image;
        TinyImageFormat  format;
        TextureDimension dimension;
        uint8_t          baseArrayLayer;
        uint8_t          layerCount;
        uint8_t          baseMipLevel;
        uint8_t          levelCount;
        friend inline bool operator==(const TextureViewKey& k1, const TextureViewKey& k2) {
            if (k1.image != k2.image) return false;
            if (k1.format != k2.format) return false;
            if (k1.dimension != k2.dimension) return false;
            if (k1.baseArrayLayer != k2.baseArrayLayer) return false;
            if (k1.layerCount != k2.layerCount) return false;
            if (k1.baseMipLevel != k2.baseMipLevel) return false;
            if (k1.levelCount != k2.levelCount) return false;
            return true;
        }
    };
    struct TextureKey {
        int32_t          id;
        uint16_t	     width;
        uint16_t	     height;
        uint16_t         depth;
        TinyImageFormat  format;
        SampleCount      sampleCount;
        TextureDimension dimension;
        uint8_t          mipLevel;
        uint8_t          arraySize;
        friend inline bool operator==(const TextureKey& k1, const TextureKey& k2) {
            if (k1.format != k2.format) return false;
            if (k1.dimension != k2.dimension) return false;
            if (k1.width != k2.width) return false;
            if (k1.height != k2.height) return false;
            if (k1.arraySize != k2.arraySize) return false;
            if (k1.mipLevel != k2.mipLevel) return false;
            if (k1.sampleCount != k2.sampleCount) return false;
            if (k1.depth != k2.depth) return false;
            return true;
        }
    };
    struct TextureDesc : TextureKey {
        ImagePtr       image;
        void* pData;//api数据 比如 vulkan 包含了内存信息，删除图像时要用到
        ResourceState  state;
        TextureUsage   usage;//这个字段是包含关系，即使不同，也可以指向相同的图像
        //MSAA 多重采样解析纹理
        TextureDesc Resolve(uint32_t id = 0) {
            TextureDesc resolve{ *(TextureKey*)this };
            resolve.id = id;
            resolve.sampleCount = SampleCount::SAMPLE_COUNT_1;
            return resolve;
        }
        TextureViewKey ToTextureView() const {
            TextureViewKey desc{};
            desc.image = image;
            desc.format = format;
            desc.baseArrayLayer = 0;
            desc.baseMipLevel = 0;
            desc.layerCount = 1;
            desc.levelCount = 1;
            desc.dimension = dimension;
            return desc;
        }
        TextureBarrier ToBarrier(ResourceState from, ResourceState to)const;
    };
    struct TextureBarrier
    {
        TextureDesc   mTexture;
        ResourceState mSrcState;
        ResourceState mDstState;
        uint8_t       mBeginOnly : 1;
        uint8_t       mEndOnly : 1;
        uint8_t       mAcquire : 1;
        uint8_t       mRelease : 1;
        uint8_t       mQueueType : 5;
        /// Specifiy whether following barrier targets particular subresource
        uint8_t       mSubresourceBarrier : 1;
        /// Following values are ignored if mSubresourceBarrier is false
        uint8_t       mMipLevel : 7;
        uint16_t      mArrayLayer;
    };
    typedef struct BufferBarrier
    {
        //Buffer*       pBuffer;
        ResourceState mSrcState;
        ResourceState mDstState;
        uint8_t       mBeginOnly : 1;
        uint8_t       mEndOnly : 1;
    } BufferBarrier;
    inline BufferBarrier BufferDesc::ToBarrier(ResourceState from, ResourceState to)const
    {
        BufferBarrier barrier{};
        barrier.mSrcState = from;
        barrier.mDstState = to;
        return barrier;
    }
    inline TextureBarrier TextureDesc::ToBarrier(ResourceState from, ResourceState to)const
    {
        TextureBarrier barrier{};
        barrier.mSrcState = from;
        barrier.mDstState = to;
        barrier.mTexture = *this;
        return barrier;
    }
    struct ResourceBarrierDesc {
        const BufferBarrier*  pBufferBarriers;
        uint32_t              bufferBarriersCount;
        const TextureBarrier* pTextureBarriers;
        uint32_t              textureBarriersCount;
    };
    /**
     * Parameters of a render pass.
     */
    struct RenderPassParams {
        uint8_t  passMask;
        uint8_t  subpassMask;
        uint8_t  sampleMask;
        uint8_t  clear;
        uint8_t  discardStart;//不关心加载内容
        uint8_t  discardEnd;//数据存储在访问最快的地方，只能在RenderPass内部用，后面就找不到了（不在附件里）
        Vector2  depthRange{ 0.f, 1.f };    //!< depth range for this pass
        Vector4  clearColor = { 0.f, 0.f, 0.f, 1.f };
        float    clearDepth;
        uint32_t clearStencil;
    };
    struct TextureUpdateArgs {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
        uint32_t mipLevel;
        const void* data;
    };
}
#include "meta/hash.h"
namespace std {
    template<>
    struct hash<api::TextureViewKey>
    {
        size_t operator()(const api::TextureViewKey& key) const noexcept
        {
            return meta::MurmurHashFn(key);
        }
    };
    template<>
    struct hash<api::TextureKey>
    {
        size_t operator()(const api::TextureKey& key) const noexcept
        {
            return meta::MurmurHashFn(key);
        }
    };
    template<>
    struct hash<api::TextureSampler>
    {
        size_t operator()(const api::TextureSampler& key) const noexcept
        {
            return std::hash<int>{}(*(int*)&key);
        }
    };
}