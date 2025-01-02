#include "meta/enum.h"
#include "vkn/vulkan_api_help.h"
namespace vkn {
    VkImageLayout vkApiGetAttachmentLayout(VkFormat format, bool includeStencilBit)
    {
        switch (format)
        {
            // Depth
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            // Stencil
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            // Depth/stencil
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            if (includeStencilBit)
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            // Assume everything else is Color
        default:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }
    VkImageLayout vkApiGetImageLayout(ResourceState layout) {
        switch (layout) {
        case ResourceState::UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case ResourceState::READ_WRITE:
            return VK_IMAGE_LAYOUT_GENERAL;
        case ResourceState::READ_ONLY:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ResourceState::TRANSFER_SRC:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ResourceState::TRANSFER_DST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ResourceState::DEPTH_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ResourceState::DEPTH_SAMPLER:
            return VK_IMAGE_LAYOUT_GENERAL;
        case ResourceState::PRESENT:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case ResourceState::COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }
    VkImageMemoryBarrier vkApiGetTextureTransition(VkPipelineStageFlags& mSrcStage, VkPipelineStageFlags& mDstStage, const TextureBarrier& barrier) {
        VkAccessFlags srcAccessMask, dstAccessMask;
        VkPipelineStageFlags srcStage, dstStage;
        VkImageAspectFlags   aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        switch (barrier.mSrcState) {
        case ResourceState::UNDEFINED:
            srcAccessMask = 0;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case ResourceState::COLOR_ATTACHMENT:
            srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case ResourceState::READ_WRITE:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case ResourceState::READ_ONLY:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case ResourceState::TRANSFER_SRC:
            srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case ResourceState::TRANSFER_DST:
            srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case ResourceState::DEPTH_ATTACHMENT:
            srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case ResourceState::DEPTH_SAMPLER:
            srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case ResourceState::PRESENT:
            srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        }

        switch (barrier.mDstState) {
        case ResourceState::COLOR_ATTACHMENT:
            dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case ResourceState::READ_WRITE:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case ResourceState::READ_ONLY:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            break;
        case ResourceState::TRANSFER_SRC:
            dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case ResourceState::TRANSFER_DST:
            dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case ResourceState::DEPTH_ATTACHMENT:
            dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        case ResourceState::DEPTH_SAMPLER:
            dstAccessMask =
                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        case ResourceState::PRESENT:
        case ResourceState::UNDEFINED:
            dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        }
        mSrcStage |= srcStage;
        mDstStage |= dstStage;
        VkImageSubresourceRange subresources{
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        return VkImageMemoryBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = vkApiGetImageLayout(barrier.mSrcState),
        .newLayout = vkApiGetImageLayout(barrier.mDstState),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = (VkImage)barrier.mTexture.image,
        .subresourceRange = subresources
        };
    }
    VkImageAspectFlags vkApiGetImageAspectMask(VkFormat format, bool includeStencilBit)
    {
        VkImageAspectFlags result = 0;
        switch (format)
        {
            // Depth
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
            // Stencil
        case VK_FORMAT_S8_UINT:
            result = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
            // Depth/stencil
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (includeStencilBit)
                result |= VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
            // Assume everything else is Color
        default:
            result = VK_IMAGE_ASPECT_COLOR_BIT;
            break;
        }
        return result;
    }
    VkImageUsageFlags vkApiGetImageUsageFlags(TextureUsage usage)
    {
        VkImageUsageFlags usageFlags = 0;
        if (any(usage & TextureUsage::COLOR_ATTACHMENT))
            usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT))
            usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (any(usage & TextureUsage::SAMPLEABLE))
            usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (any(usage & TextureUsage::BLIT_DST))
            usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (any(usage & TextureUsage::BLIT_SRC))
            usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        return usageFlags;
    }
    VkImageViewType vkApiGetImageViewType(TextureDimension dimension, uint32_t arraySize)
    {
        if (dimension == TextureDimension::TEX_NULL || any(dimension & TextureDimension::TEX_1D)) {
            return arraySize > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
        }
        if (any(dimension & TextureDimension::TEX_CUBE)) {
            return arraySize > 1 ?  VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        }
        if (any(dimension & TextureDimension::TEX_2D)) {
            return arraySize > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        }
        if (any(dimension & TextureDimension::TEX_3D)) {
            return VK_IMAGE_VIEW_TYPE_3D;
        }
        return arraySize > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
    }
    VkImageType vkApiGetImageType(TextureDimension dimension)
    {
        if (any(dimension & TextureDimension::TEX_1D)) {
            return VK_IMAGE_TYPE_1D;
        }
        if (any(dimension & TextureDimension::TEX_2D)) {
            return VK_IMAGE_TYPE_2D;
        }
        if (any(dimension & TextureDimension::TEX_3D)) {
            return VK_IMAGE_TYPE_3D;
        }
        return VK_IMAGE_TYPE_MAX_ENUM;
    }
    VkImageCreateFlags vkApiGetImageCreateFlag(TextureDimension dimension, uint32_t arraySize)
    {
        VkImageCreateFlags flag{};
        if (any(dimension & TextureDimension::TEX_CUBE)) {
            flag |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }
        if (arraySize > 1) {
            flag |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
        }
        return flag;
    }
    VkSampleCountFlagBits vkApiGetSmpleCountFlag(SampleCount sample)
    {
        return (VkSampleCountFlagBits)sample;
    }
    VkDescriptorType vkApiGetDescriptorType(ShaderDescriptorType type)
    {
        switch (type) {
        case ShaderDescriptorType::SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case ShaderDescriptorType::UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }
    VkShaderStageFlags vkApiGetShaderStageFlags(ShaderStage stage)
    {
        VkShaderStageFlags flags{};
        if (any(stage & ShaderStage::FRAGMENT)) {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (any(stage & ShaderStage::VERTEX)) {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        return flags;
    }
    using api::SamplerWrapMode;
    using api::SamplerMinFilter;
    using api::SamplerMagFilter;
    using api::SamplerCompareMode;
    using api::SamplerCompareFunc;
    constexpr inline VkSamplerAddressMode getWrapMode(SamplerWrapMode mode) noexcept {
        switch (mode) {
        case SamplerWrapMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerWrapMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerWrapMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        }
    }

    constexpr inline VkFilter getFilter(SamplerMinFilter filter) noexcept {
        switch (filter) {
        case SamplerMinFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR:
            return VK_FILTER_LINEAR;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
            return VK_FILTER_LINEAR;
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return VK_FILTER_LINEAR;
        }
    }

    constexpr inline VkFilter getFilter(SamplerMagFilter filter) noexcept {
        switch (filter) {
        case SamplerMagFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMagFilter::LINEAR:
            return VK_FILTER_LINEAR;
        }
    }

    constexpr inline VkSamplerMipmapMode getMipmapMode(SamplerMinFilter filter) noexcept {
        switch (filter) {
        case SamplerMinFilter::NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }

    constexpr inline float getMaxLod(SamplerMinFilter filter) noexcept {
        switch (filter) {
        case SamplerMinFilter::NEAREST:
        case SamplerMinFilter::LINEAR:
            // The Vulkan spec recommends a max LOD of 0.25 to "disable" mipmapping.
            // See "Mapping of OpenGL to Vulkan filter modes" in the VK Spec.
            return 0.25f;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return VK_LOD_CLAMP_NONE;
        }
    }

    constexpr inline VkBool32 getCompareEnable(SamplerCompareMode mode) noexcept {
        return mode == SamplerCompareMode::NONE ? VK_FALSE : VK_TRUE;
    }
    VkCompareOp getCompareOp(SamplerCompareFunc func) {
        using Compare = SamplerCompareFunc;
        switch (func) {
        case Compare::LE: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Compare::GE: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Compare::L:  return VK_COMPARE_OP_LESS;
        case Compare::G:  return VK_COMPARE_OP_GREATER;
        case Compare::E:  return VK_COMPARE_OP_EQUAL;
        case Compare::NE: return VK_COMPARE_OP_NOT_EQUAL;
        case Compare::A:  return VK_COMPARE_OP_ALWAYS;
        case Compare::N:  return VK_COMPARE_OP_NEVER;
        }
    }
    VkSamplerCreateInfo vkApiGetSamplerCreateInfo(TextureSampler sampler)
    {
        VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = getFilter(sampler.filterMag),
            .minFilter = getFilter(sampler.filterMin),
            .mipmapMode = getMipmapMode(sampler.filterMin),
            .addressModeU = getWrapMode(sampler.wrapS),
            .addressModeV = getWrapMode(sampler.wrapT),
            .addressModeW = getWrapMode(sampler.wrapR),
            .anisotropyEnable = sampler.anisotropyLog2 == 0 ? 0u : 1u,
            .maxAnisotropy = (float)(1u << sampler.anisotropyLog2),
            .compareEnable = getCompareEnable(sampler.compareMode),
            .compareOp = getCompareOp(sampler.compareFunc),
            .minLod = 0.0f,
            .maxLod = getMaxLod(sampler.filterMin),
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };
        return samplerInfo;
    }
    VkBufferUsageFlagBits vkApiGetBufferUsage(BufferUsage usage)
    {
        switch (usage)
        {
        case api::BufferUsage::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case api::BufferUsage::INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case api::BufferUsage::UNIFORM:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case api::BufferUsage::SHADER_STORAGE:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case api::BufferUsage::TRANSFER_SRC:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        default:
            return {};
        }
    }
    VkMemoryPropertyFlags vkApiGetMemoryFlags(ResourceMemoryUsage usageFlags) {
        VkMemoryPropertyFlags flags = 0;
        if (any(usageFlags & ResourceMemoryUsage::HOST_VISIBLE)) {
            flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        }
        if (any(usageFlags & ResourceMemoryUsage::GPU_ONLY)) {
            flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }
        if (any(usageFlags & ResourceMemoryUsage::HOST_COHERENT)) {
            flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        if (any(usageFlags & ResourceMemoryUsage::LAZY_ALLOCATED)) {
            flags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        }
        return flags;
    }

    VmaMemoryUsage vkApiGetMemoryUsage(ResourceMemoryUsage usageFlags) {
        if (any(usageFlags & ResourceMemoryUsage::GPU_ONLY)) {
            return VMA_MEMORY_USAGE_GPU_ONLY;
        }
        if (any(usageFlags & ResourceMemoryUsage::CPU_ONLY)) {
            return VMA_MEMORY_USAGE_CPU_ONLY;
        }
        if (any(usageFlags & ResourceMemoryUsage::CPU_TO_GPU)) {
            return VMA_MEMORY_USAGE_CPU_TO_GPU;
        }
        if (any(usageFlags & ResourceMemoryUsage::GPU_TO_CPU)) {
            return VMA_MEMORY_USAGE_GPU_TO_CPU;
        }
        return VMA_MEMORY_USAGE_UNKNOWN;  // 默认
    }
}