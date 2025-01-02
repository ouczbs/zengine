#pragma once
#include <Windows.h>
#include <functional>
#define VK_NO_PROTOTYPES
#include "volk/volk.h"
#include "vma/vk_mem_alloc.h"
#include "render/render_type.h"
#include "render/type.h"
#define Z_RENDER_DEBUG 1
#define VKN_MACRO_BEGIN \
        __pragma(warning(push)) \
        __pragma(warning(disable:4127)) \
        do {
#define VKN_MACRO_END \
        } while(false) \
        __pragma(warning(pop))
#define V(exp) \
    VKN_MACRO_BEGIN \
        VkResult err_ = (exp); \
        assert(err_ == VK_SUCCESS); \
    VKN_MACRO_END
namespace vkn {
	using pmr::Name;
	using pmr::table;
	using std::vector;
	using std::string_view;
	class CommandBuffer;
	using voidFn = std::function<void()>;
	using commandFn = std::function<void(CommandBuffer& cmd)>;
	using api::ResourceBarrierDesc;
	using api::TextureViewKey;
	using api::ImageViewPtr;
	using api::ImagePtr;
	using api::TextureDimension;
	using api::SampleCount;
	using api::TextureSampler;
	using api::SamplerPtr;
	using api::ResourceState;
	using api::TextureDesc;
	using api::BufferDesc;
	using api::ShaderDescriptorLayout;
	using api::ShaderDescriptorType;
	using api::BufferUsage;
	using api::MaterialInfo;
	using api::TextureUpdateArgs;

	constexpr string_view VulkanEngineName = "vulkan";
	constexpr uint8_t  MAX_SUPPORTED_RENDER_TARGET_COUNT = 8u;
	constexpr uint32_t SHADER_MODULE_COUNT = 2;
	constexpr uint32_t VERTEX_ATTRIBUTE_COUNT = 16;
	constexpr uint32_t MAX_BUFFER_BIND_NUM = 16;
	constexpr uint32_t MAX_SHADER_DESCRIPTOR_SET_COUNT = 3;
	using VkDescriptorSetLayoutList = VkDescriptorSetLayout[MAX_SHADER_DESCRIPTOR_SET_COUNT];
	using VkDescriptorSetList = VkDescriptorSet[MAX_SHADER_DESCRIPTOR_SET_COUNT];
	struct MeshVAO
	{
		uint32_t indexCount{0}; // 索引数量
		uint32_t vertexCount{ 0 }; // 顶点数量
		VkBuffer indexBuffer;
		VkBuffer vertexBuffer;
		VmaAllocation indexAllocation;
		VmaAllocation vertexAllocation;
	};
	//不需要支持所有，只要支持常见的就行
	struct RenderPassKey {
		TinyImageFormat colorFormat[MAX_SUPPORTED_RENDER_TARGET_COUNT];
		uint8_t			depthMask;
		uint8_t clear; // 1 bytes
		uint8_t discardStart; // 1 bytes
		uint8_t discardEnd; // 1 bytes
		SampleCount samples;
		uint8_t  sampleMask;// 1 byte
		uint8_t  passMask;// 1 byte
		uint8_t  subpassMask;// 1 byte
		operator size_t() const{
			return meta::MurmurHashFn(*this);
		}
	};
	struct RenderPassInfo {
		VkRenderPass  pass;
		RenderPassKey config;
		vector<VkSemaphore>     semaphores;
		vector<VkCommandBuffer> commands;
	};
	struct FramebufferKey {
		VkRenderPass pass;
		VkImageView	 imageViews[MAX_SUPPORTED_RENDER_TARGET_COUNT * 2 + 1];
		uint32_t	 attachmentCount;
		uint32_t	 width;
		uint32_t	 height;
		uint32_t	 layers;
		friend inline bool operator==(const FramebufferKey& k1, const FramebufferKey& k2) {
			if (k1.pass != k2.pass) return false;
			if (k1.attachmentCount != k2.attachmentCount) return false;
			for (int i = 0; i < k1.attachmentCount; i++) {
				if (k1.imageViews[i] != k2.imageViews[i]) return false;
			}
			if (k1.height != k2.height) return false;
			if (k1.width != k2.width) return false;
			if (k1.layers != k2.layers) return false;
			return true;
		}
	};
	// Equivalent to VkVertexInputAttributeDescription but half as big.
	struct VertexInputAttributeDescription {
		VertexInputAttributeDescription& operator=(const VkVertexInputAttributeDescription& that) {
			location = that.location;
			binding = that.binding;
			format = that.format;
			offset = that.offset;
			return *this;
		}
		operator VkVertexInputAttributeDescription() const {
			return { location, binding, VkFormat(format), offset };
		}
		uint8_t     location;
		uint8_t     binding;
		uint16_t    format;
		uint32_t    offset;
	};
	// Equivalent to VkVertexInputBindingDescription but not as big.
	struct VertexInputBindingDescription {
		VertexInputBindingDescription& operator=(const VkVertexInputBindingDescription& that) {
			binding = that.binding;
			stride = that.stride;
			inputRate = that.inputRate;
			return *this;
		}
		operator VkVertexInputBindingDescription() const {
			return { binding, stride, (VkVertexInputRate)inputRate };
		}
		uint16_t binding;
		uint16_t inputRate;
		uint32_t stride;
	};
	struct VulkanPipeline
	{
		Name name; // For debug
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetList descList{};
		VkDescriptorSetLayoutList descLayoutList{};
		uint32_t descCount = 0;
		bool inUse = false;
	};
}
namespace std {
	template<>
	struct hash<vkn::FramebufferKey>
	{
		size_t operator()(const vkn::FramebufferKey& key) const noexcept
		{
			return meta::MurmurHashFn(key);
		}
	};
}