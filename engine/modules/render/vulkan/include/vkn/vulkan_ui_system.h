#pragma once
#include "vkn/vulkan_api.h"
#include "vkn/wrapper/device.h"
#include "ui/ui_render_system.h"
#include "editor/editor_system.h"
#include "render/graph/frame_graph.h"
#include <NsCore/HashMap.h>
namespace vkn {
	using Noesis::BaseVector;
	using api::UITexture;
	using api::FrameGraph;
	using api::RenderPassContext;
	using api::RenderPassBuilder;
	using api::RenderEditorContext;
	struct Layout
	{
		uint32_t signature;
		VkDescriptorSetLayout setLayout;
		VkPipelineLayout pipelineLayout;
	};
	struct PiplineHashKey {
		void* renderPass;
		uint32_t shader;
		uint32_t id;
		size_t hash() const{
			return meta::MurmurHashFn(*this);
		}
	};
	class VulkanUISystem : public api::UIRenderSystem {
	private:
		Device*				mDevice;
		uint8_t             mWaitTransferCount;
		bool				mIsLinearRendering;
		bool				mHasExtendedDynamicState;
		uint32_t			mLastTextureHashValue{1};
		uint32_t			mLastBufferHashValue{1};
		uint32_t			mFrameNumber;
		uint32_t			mSafeFrameNumber;
		uint32_t			mMinUniformBufferOffsetAlignment;
		uint32_t			mCachedStencilRef;
		VkBuffer			mCachedIndexBuffer;
		VkCommandBuffer		mCommandBuffer = VK_NULL_HANDLE;
		VkPipeline			mCachedPipeline = VK_NULL_HANDLE;
		VkPipelineCache		mPipelineCache = VK_NULL_HANDLE;
		VkRenderPass		mRenderPass = VK_NULL_HANDLE;
		VkRenderPass		mRenderPassNoClear = VK_NULL_HANDLE;
		VkDescriptorPool	mDescriptorPool = VK_NULL_HANDLE;
		uint32_t									mCachedConstantHash[4];
		VkSampler									mSamplers[64];
		Noesis::HashMap<uint32_t, Layout, 16>		mLayoutMap;
		Noesis::HashMap<uint32_t, uint32_t>			mPipelineMap;
		Noesis::HashMap<uint32_t, VkDescriptorSet>	mDescriptorSetMap;
		Noesis::Vector<Noesis::Pair<VkDescriptorPool, uint64_t>> mFreeDescriptorPools;
		Noesis::Vector<VkPipeline>					mPipelines;
		VkShaderModule		mVertexShaders[Noesis::Shader::Vertex::Count];
		VkShaderModule		mPixelShaders[Noesis::Shader::Count];
		Layout				mLayouts[Noesis::Shader::Count];
	public:
		inline static Name	UIPassName{ "UIPass" };
		void InitNoesisRender(bool linearRendering, bool stereoSupport, SampleCount sampleCount)override;
		void BeginRender(api::RenderContext* context) override;
		void DrawBatch(const Noesis::Batch& batch) override;
	public:
		void OnBeginRenderFrame(FrameGraph& graph, uint32_t frame);
		static void Setup(FrameGraph& graph, RenderPassBuilder& builder);
		static void Execute(FrameGraph&, RenderPassContext&);
	public:
		void SetBuffers(const Noesis::Batch& batch);
		void BindDescriptors(const Noesis::Batch& batch, const Layout& layout);
		void UploadUniforms(uint32_t i, const Noesis::UniformData* data, uint32_t& hash, BaseVector<uint32_t>& offsets);
		void TextureHash(uint32_t& hash, UITexture* texture, uint8_t sampler);
		void FillBufferInfo(uint32_t i, const Noesis::UniformData* data, VkDescriptorSet set,
			BaseVector<VkDescriptorBufferInfo>& buffers, BaseVector<VkWriteDescriptorSet>& writes,
			uint32_t& binding);
		void FillImageInfo(UITexture* texture, uint8_t sampler, VkDescriptorSet set,
			BaseVector<VkDescriptorImageInfo>& images, BaseVector<VkWriteDescriptorSet>& writes,uint32_t& binding);
		void BindPipeline(const Noesis::Batch& batch);
		void SetStencilRef(uint32_t stencilRef);
	public:
		void CreateRenderPass(TinyImageFormat format, VkSampleCountFlagBits sampleCount);
		void CreateLayouts();
		void CreateLayout(uint32_t signature, Layout& layout);
		void LoadShaders(bool stereoSupport);
		void CreatePipelines(VkSampleCountFlagBits sampleCount);
		void CreatePipelines(uint8_t shader,VkShaderModule psModule, VkPipelineLayout layout, VkSampleCountFlagBits sampleCount);
		void CreatePipelines(uint8_t shader_, VkGraphicsPipelineCreateInfo& pipelineInfo);
		void CreateSamplers();
		void CreateDescriptorPool();
	};
}