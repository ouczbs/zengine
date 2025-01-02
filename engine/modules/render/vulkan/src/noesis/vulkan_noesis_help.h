#include "vulkan_noesis_shaders.h"
#include "NsRender/RenderDevice.h"
using namespace Noesis;
// Root signature flags
static const uint32_t VS_CB0 = 1;
static const uint32_t VS_CB1 = 2;
static const uint32_t PS_CB0 = 4;
static const uint32_t PS_CB1 = 8;
static const uint32_t PS_T0 = 16;
static const uint32_t PS_T1 = 32;
static const uint32_t PS_T2 = 64;
static const uint32_t PS_T3 = 128;
static const uint32_t PS_T4 = 256;

struct ShaderVS
{
    const char* label;
    uint32_t start;
    uint32_t size;
};

#ifdef NS_PLATFORM_ANDROID
#define VSHADER(n) case Shader::Vertex::n: \
        return stereo ? \
            ShaderVS { #n, n##_Multiview_VS_Start, n##_Multiview_VS_Size } : \
            ShaderVS { #n, n##_VS_Start, n##_VS_Size };

#define VSHADER_SRGB(n) case Shader::Vertex::n: \
        return stereo ? \
            (sRGB ? \
                ShaderVS{ #n"_sRGB", n##_SRGB_Multiview_VS_Start, n##_SRGB_Multiview_VS_Size } : \
                ShaderVS{ #n, n##_Multiview_VS_Start, n##_Multiview_VS_Size }) : \
            (sRGB ? \
                ShaderVS{ #n"_sRGB", n##_SRGB_VS_Start, n##_SRGB_VS_Size } : \
                ShaderVS{ #n, n##_VS_Start, n##_VS_Size });
#else
#define VSHADER(n) case Shader::Vertex::n: \
        return stereo ? \
            ShaderVS { #n, n##_Layer_VS_Start, n##_Layer_VS_Size } : \
            ShaderVS { #n, n##_VS_Start, n##_VS_Size };

#define VSHADER_SRGB(n) case Shader::Vertex::n: \
        return stereo ? \
            (sRGB ? \
                ShaderVS{ #n"_sRGB", n##_SRGB_Layer_VS_Start, n##_SRGB_Layer_VS_Size } : \
                ShaderVS{ #n, n##_Layer_VS_Start, n##_Layer_VS_Size }) : \
            (sRGB ? \
                ShaderVS{ #n"_sRGB", n##_SRGB_VS_Start, n##_SRGB_VS_Size } : \
                ShaderVS{ #n, n##_VS_Start, n##_VS_Size });
#endif

static auto ShadersVS = [](uint32_t shader, bool sRGB, bool stereo)
{
    switch (shader)
    {
        VSHADER(Pos)
            VSHADER_SRGB(PosColor)
            VSHADER(PosTex0)
            VSHADER(PosTex0Rect)
            VSHADER(PosTex0RectTile)
            VSHADER_SRGB(PosColorCoverage)
            VSHADER(PosTex0Coverage)
            VSHADER(PosTex0CoverageRect)
            VSHADER(PosTex0CoverageRectTile)
            VSHADER_SRGB(PosColorTex1_SDF)
            VSHADER(PosTex0Tex1_SDF)
            VSHADER(PosTex0Tex1Rect_SDF)
            VSHADER(PosTex0Tex1RectTile_SDF)
            VSHADER_SRGB(PosColorTex1)
            VSHADER(PosTex0Tex1)
            VSHADER(PosTex0Tex1Rect)
            VSHADER(PosTex0Tex1RectTile)
            VSHADER_SRGB(PosColorTex0Tex1)
            VSHADER(PosTex0Tex1_Downsample)
            VSHADER_SRGB(PosColorTex1Rect)
            VSHADER_SRGB(PosColorTex0RectImagePos)

    default: NS_ASSERT_UNREACHABLE;
    }
};

struct ShaderPS
{
    const char* label;
    uint32_t start;
    uint32_t size;
    uint32_t signature;
};

#define PSHADER(n, s) case Shader::n: return ShaderPS { #n, n##_PS_Start, n##_PS_Size, s };

static auto ShadersPS = [](uint32_t shader)
{
    switch (shader)
    {
        PSHADER(RGBA, VS_CB0 | PS_CB0)
            PSHADER(Mask, VS_CB0)
            PSHADER(Clear, VS_CB0)

            PSHADER(Path_Solid, VS_CB0)
            PSHADER(Path_Linear, VS_CB0 | PS_CB0 | PS_T1)
            PSHADER(Path_Radial, VS_CB0 | PS_CB0 | PS_T1)
            PSHADER(Path_Pattern, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_Pattern_Clamp, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_Pattern_Repeat, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_Pattern_MirrorU, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_Pattern_MirrorV, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_Pattern_Mirror, VS_CB0 | PS_CB0 | PS_T0)

            PSHADER(Path_AA_Solid, VS_CB0)
            PSHADER(Path_AA_Linear, VS_CB0 | PS_CB0 | PS_T1)
            PSHADER(Path_AA_Radial, VS_CB0 | PS_CB0 | PS_T1)
            PSHADER(Path_AA_Pattern, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_AA_Pattern_Clamp, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_AA_Pattern_Repeat, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_AA_Pattern_MirrorU, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_AA_Pattern_MirrorV, VS_CB0 | PS_CB0 | PS_T0)
            PSHADER(Path_AA_Pattern_Mirror, VS_CB0 | PS_CB0 | PS_T0)

            PSHADER(SDF_Solid, VS_CB0 | VS_CB1 | PS_T3)
            PSHADER(SDF_Linear, VS_CB0 | VS_CB1 | PS_CB0 | PS_T1 | PS_T3)
            PSHADER(SDF_Radial, VS_CB0 | VS_CB1 | PS_CB0 | PS_T1 | PS_T3)
            PSHADER(SDF_Pattern, VS_CB0 | VS_CB1 | PS_CB0 | PS_T0 | PS_T3)
            PSHADER(SDF_Pattern_Clamp, VS_CB0 | VS_CB1 | PS_CB0 | PS_T0 | PS_T3)
            PSHADER(SDF_Pattern_Repeat, VS_CB0 | VS_CB1 | PS_CB0 | PS_T0 | PS_T3)
            PSHADER(SDF_Pattern_MirrorU, VS_CB0 | VS_CB1 | PS_CB0 | PS_T0 | PS_T3)
            PSHADER(SDF_Pattern_MirrorV, VS_CB0 | VS_CB1 | PS_CB0 | PS_T0 | PS_T3)
            PSHADER(SDF_Pattern_Mirror, VS_CB0 | VS_CB1 | PS_CB0 | PS_T0 | PS_T3)

            PSHADER(Opacity_Solid, VS_CB0 | PS_T2)
            PSHADER(Opacity_Linear, VS_CB0 | PS_CB0 | PS_T1 | PS_T2)
            PSHADER(Opacity_Radial, VS_CB0 | PS_CB0 | PS_T1 | PS_T2)
            PSHADER(Opacity_Pattern, VS_CB0 | PS_CB0 | PS_T0 | PS_T2)
            PSHADER(Opacity_Pattern_Clamp, VS_CB0 | PS_CB0 | PS_T0 | PS_T2)
            PSHADER(Opacity_Pattern_Repeat, VS_CB0 | PS_CB0 | PS_T0 | PS_T2)
            PSHADER(Opacity_Pattern_MirrorU, VS_CB0 | PS_CB0 | PS_T0 | PS_T2)
            PSHADER(Opacity_Pattern_MirrorV, VS_CB0 | PS_CB0 | PS_T0 | PS_T2)
            PSHADER(Opacity_Pattern_Mirror, VS_CB0 | PS_CB0 | PS_T0 | PS_T2)

            PSHADER(Upsample, VS_CB0 | PS_T0 | PS_T2)
            PSHADER(Downsample, VS_CB0 | PS_T0)

            PSHADER(Shadow, VS_CB0 | PS_CB1 | PS_T2 | PS_T4)
            PSHADER(Blur, VS_CB0 | PS_CB1 | PS_T2 | PS_T4)

    default: return ShaderPS{};
    }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
static void FillLayoutBindings(uint32_t buffersVS, uint32_t buffersPS, uint32_t textures,
    BaseVector<VkDescriptorSetLayoutBinding>& v)
{
    NS_ASSERT(buffersVS + buffersPS <= 4);
    NS_ASSERT(textures <= 3);

    uint32_t binding = 0;

    for (uint32_t i = 0; i < buffersVS; i++)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.descriptorCount = 1;
        layoutBinding.binding = binding++;
        layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

        v.PushBack(layoutBinding);
    }

    for (uint32_t i = 0; i < buffersPS; i++)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.descriptorCount = 1;
        layoutBinding.binding = binding++;
        layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

        v.PushBack(layoutBinding);
    }

    for (uint32_t i = 0; i < textures; i++)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.descriptorCount = 1;
        layoutBinding.binding = binding++;
        layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        v.PushBack(layoutBinding);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static VkFormat Format(uint32_t type)
{
    switch (type)
    {
    case Shader::Vertex::Format::Attr::Type::Float: return VK_FORMAT_R32_SFLOAT;
    case Shader::Vertex::Format::Attr::Type::Float2: return VK_FORMAT_R32G32_SFLOAT;
    case Shader::Vertex::Format::Attr::Type::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Shader::Vertex::Format::Attr::Type::UByte4Norm: return VK_FORMAT_R8G8B8A8_UNORM;
    case Shader::Vertex::Format::Attr::Type::UShort4Norm: return VK_FORMAT_R16G16B16A16_UNORM;
    default: NS_ASSERT_UNREACHABLE;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void FillVertexAttributes(uint32_t format, BaseVector<VkVertexInputAttributeDescription>& v)
{
    uint32_t attributes = AttributesForFormat[format];
    uint32_t offset = 0;

    for (uint32_t i = 0; i < Shader::Vertex::Format::Attr::Count; i++)
    {
        if (attributes & (1 << i))
        {
            VkVertexInputAttributeDescription& attr = v.EmplaceBack();
            memset(&attr, 0, sizeof(attr));

            attr.binding = 0;
            attr.location = i;
            attr.format = Format(TypeForAttr[i]);
            attr.offset = offset;

            offset += SizeForType[TypeForAttr[i]];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void RasterizerInfo(VkPipelineRasterizationStateCreateInfo& info, RenderState state,
    const VkPhysicalDeviceFeatures& features)
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.lineWidth = 1.0f;
    info.cullMode = VK_CULL_MODE_NONE;
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;

    if (state.f.wireframe && features.fillModeNonSolid)
    {
        info.polygonMode = VK_POLYGON_MODE_LINE;
    }
    else
    {
        info.polygonMode = VK_POLYGON_MODE_FILL;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void DepthStencilInfo(VkPipelineDepthStencilStateCreateInfo& info, RenderState state)
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthWriteEnable = VK_FALSE;
    info.depthBoundsTestEnable = VK_FALSE;
    info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;

    info.front.failOp = VK_STENCIL_OP_KEEP;
    info.front.depthFailOp = VK_STENCIL_OP_KEEP;
    info.front.writeMask = 0xFF;
    info.front.compareMask = 0XFF;

    info.back.failOp = VK_STENCIL_OP_KEEP;
    info.back.depthFailOp = VK_STENCIL_OP_KEEP;
    info.back.writeMask = 0xFF;
    info.back.compareMask = 0XFF;

    switch (state.f.stencilMode)
    {
        case StencilMode::Disabled:
        {
            info.depthTestEnable = VK_FALSE;
            info.stencilTestEnable = VK_FALSE;
            info.front.compareOp = VK_COMPARE_OP_EQUAL;
            info.back.compareOp = VK_COMPARE_OP_EQUAL;
            info.front.passOp = VK_STENCIL_OP_KEEP;
            info.back.passOp = VK_STENCIL_OP_KEEP;
            break;
        }
        case StencilMode::Equal_Keep:
        {
            info.depthTestEnable = VK_FALSE;
            info.stencilTestEnable = VK_TRUE;
            info.front.compareOp = VK_COMPARE_OP_EQUAL;
            info.back.compareOp = VK_COMPARE_OP_EQUAL;
            info.front.passOp = VK_STENCIL_OP_KEEP;
            info.back.passOp = VK_STENCIL_OP_KEEP;
            break;
        }
        case StencilMode::Equal_Incr:
        {
            info.depthTestEnable = VK_FALSE;
            info.stencilTestEnable = VK_TRUE;
            info.front.compareOp = VK_COMPARE_OP_EQUAL;
            info.back.compareOp = VK_COMPARE_OP_EQUAL;
            info.front.passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
            info.back.passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
            break;
        }
        case StencilMode::Equal_Decr:
        {
            info.depthTestEnable = VK_FALSE;
            info.stencilTestEnable = VK_TRUE;
            info.front.compareOp = VK_COMPARE_OP_EQUAL;
            info.back.compareOp = VK_COMPARE_OP_EQUAL;
            info.front.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
            info.back.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
            break;
        }
        case StencilMode::Clear:
        {
            info.depthTestEnable = VK_FALSE;
            info.stencilTestEnable = VK_TRUE;
            info.front.compareOp = VK_COMPARE_OP_ALWAYS;
            info.back.compareOp = VK_COMPARE_OP_ALWAYS;
            info.front.passOp = VK_STENCIL_OP_ZERO;
            info.back.passOp = VK_STENCIL_OP_ZERO;
            break;
        }
        case StencilMode::Disabled_ZTest:
        {
            info.depthTestEnable = VK_TRUE;
            info.stencilTestEnable = VK_FALSE;
            info.front.compareOp = VK_COMPARE_OP_EQUAL;
            info.back.compareOp = VK_COMPARE_OP_EQUAL;
            info.front.passOp = VK_STENCIL_OP_KEEP;
            info.back.passOp = VK_STENCIL_OP_KEEP;
            break;
        }
        case StencilMode::Equal_Keep_ZTest:
        {
            info.depthTestEnable = VK_TRUE;
            info.stencilTestEnable = VK_TRUE;
            info.front.compareOp = VK_COMPARE_OP_EQUAL;
            info.back.compareOp = VK_COMPARE_OP_EQUAL;
            info.front.passOp = VK_STENCIL_OP_KEEP;
            info.back.passOp = VK_STENCIL_OP_KEEP;
            break;
        }
        default:
        {
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void BlendInfo(VkPipelineColorBlendAttachmentState& info, RenderState state)
{
    if (state.f.colorEnable)
    {
        info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        info.colorBlendOp = VK_BLEND_OP_ADD;
        info.alphaBlendOp = VK_BLEND_OP_ADD;

        switch (state.f.blendMode)
        {
            case BlendMode::Src:
            {
                info.blendEnable = VK_FALSE;
                break;
            }
            case BlendMode::SrcOver:
            {
                info.blendEnable = VK_TRUE;
                info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            }
            case BlendMode::SrcOver_Multiply:
            {
                info.blendEnable = VK_TRUE;
                info.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
                info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            }
            case BlendMode::SrcOver_Screen:
            {
                info.blendEnable = VK_TRUE;
                info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            }
            case BlendMode::SrcOver_Additive:
            {
                info.blendEnable = VK_TRUE;
                info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                break;
            }
            default:
            {
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetMinMagFilter(MinMagFilter::Enum minmag, VkSamplerCreateInfo& samplerInfo)
{
    switch (minmag)
    {
    case MinMagFilter::Nearest:
    {
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        break;
    }
    case MinMagFilter::Linear:
    {
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        break;
    }
    default:
    {
        NS_ASSERT_UNREACHABLE;
    }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetMipFilter(MipFilter::Enum mip, VkSamplerCreateInfo& samplerInfo)
{
    switch (mip)
    {
    case MipFilter::Disabled:
    {
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.maxLod = 0.0f;
        break;
    }
    case MipFilter::Nearest:
    {
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.maxLod = FLT_MAX;
        break;
    }
    case MipFilter::Linear:
    {
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.maxLod = FLT_MAX;
        break;
    }
    default:
    {
        NS_ASSERT_UNREACHABLE;
    }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void SetAddress(WrapMode::Enum mode, VkSamplerCreateInfo& samplerInfo)
{
    switch (mode)
    {
    case WrapMode::ClampToEdge:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        break;
    }
    case WrapMode::ClampToZero:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        break;
    }
    case WrapMode::Repeat:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        break;
    }
    case WrapMode::MirrorU:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        break;
    }
    case WrapMode::MirrorV:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        break;
    }
    case WrapMode::Mirror:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        break;
    }
    default:
    {
        NS_ASSERT_UNREACHABLE;
    }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
static uint32_t GetSignature(const Batch& batch)
{
    uint32_t signature = 0;

    if (batch.pattern) signature |= PS_T0;
    if (batch.ramps) signature |= PS_T1;
    if (batch.image) signature |= PS_T2;
    if (batch.glyphs) signature |= PS_T3;
    if (batch.shadow) signature |= PS_T4;
    if (batch.vertexUniforms[0].values) signature |= VS_CB0;
    if (batch.vertexUniforms[1].values) signature |= VS_CB1;
    if (batch.pixelUniforms[0].values) signature |= PS_CB0;
    if (batch.pixelUniforms[1].values) signature |= PS_CB1;

    return signature;
}