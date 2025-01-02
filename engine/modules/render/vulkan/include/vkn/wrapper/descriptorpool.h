#pragma once
#include "vkn/type.h"
#include "commandbuffer.h"
namespace vkn {
	class Device;
	class Queue;
	class DescriptorPool {
	protected:
		VkDescriptorPool mPtr;
		Device& mDevice;
	public:
		DescriptorPool(Device& device, pmr::vector<VkDescriptorPoolSize>& pPoolSizes,uint32_t maxSets);

		VkDescriptorPool Ptr() {
			return mPtr;
		}
		VkDescriptorSet Allocate(VkDescriptorSetLayout& descriptorSetLayout);
	public:
		static pmr::vector<VkDescriptorPoolSize> DefaultDescriptorPoolSize() {
			return pmr::vector<VkDescriptorPoolSize>{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8192 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8192 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 },
			};
		}
	};
}