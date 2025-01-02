#include "vkn/wrapper/descriptorpool.h"
#include "vkn/wrapper/device.h"
namespace vkn {
	DescriptorPool::DescriptorPool(Device& device, pmr::vector<VkDescriptorPoolSize>& PoolSizes, uint32_t maxSets)
		: mPtr(nullptr)
		, mDevice(device)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = PoolSizes.size();
		descriptorPoolInfo.pPoolSizes = PoolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;

		vkCreateDescriptorPool(mDevice.Ptr(), &descriptorPoolInfo, nullptr, &mPtr);
	}

	VkDescriptorSet DescriptorPool::Allocate(VkDescriptorSetLayout& descriptorSetLayout)
	{
		
		VkDescriptorSet descriptorSet;
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mPtr;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		vkAllocateDescriptorSets(mDevice.Ptr(), &allocInfo, &descriptorSet);
		return descriptorSet;
	}

}
