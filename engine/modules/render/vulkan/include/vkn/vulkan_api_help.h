#include "type.h"
namespace vkn {
	using api::TextureUsage;
	using api::ResourceState;
	using api::TextureBarrier;
	using api::ShaderDescriptorType;
	using api::ShaderStage;
	using api::ResourceMemoryUsage;
	struct VkTextureTransitionDesc {
		VkAccessFlags srcAccessMask;
		VkAccessFlags dstAccessMask;
		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		VkImageLayout mSrcState;
		VkImageLayout mDstState;
	};
	VkImageLayout		  vkApiGetAttachmentLayout(VkFormat format, bool includeStencilBit);
	VkImageLayout		  vkApiGetImageLayout(ResourceState layout);
	VkImageMemoryBarrier  vkApiGetTextureTransition(VkPipelineStageFlags& mSrcStage, VkPipelineStageFlags& mDstStage, const TextureBarrier& barrier);
	VkImageAspectFlags    vkApiGetImageAspectMask(VkFormat format, bool includeStencilBit);
	VkImageUsageFlags	  vkApiGetImageUsageFlags(TextureUsage usage);
	VkImageViewType		  vkApiGetImageViewType(TextureDimension dimension, uint32_t arraySize);
	VkImageType			  vkApiGetImageType(TextureDimension dimension);
	VkImageCreateFlags	  vkApiGetImageCreateFlag(TextureDimension dimension, uint32_t arraySize);
	VkSampleCountFlagBits vkApiGetSmpleCountFlag(SampleCount sample);

	VkDescriptorType	  vkApiGetDescriptorType(ShaderDescriptorType type);
	VkShaderStageFlags    vkApiGetShaderStageFlags(ShaderStage stage);
	VkSamplerCreateInfo   vkApiGetSamplerCreateInfo(TextureSampler sampler);

	VkBufferUsageFlagBits vkApiGetBufferUsage(BufferUsage usage);
	VkMemoryPropertyFlags vkApiGetMemoryFlags(ResourceMemoryUsage usageFlags);
	VmaMemoryUsage		  vkApiGetMemoryUsage(ResourceMemoryUsage usageFlags);
}