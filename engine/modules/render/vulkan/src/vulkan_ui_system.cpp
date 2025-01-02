#include "vkn/vulkan_ui_system.h"
#include "vkn/vulkan_api_help.h"
#include "vkn/vulkan_api.h"
#ifdef WTIH_EDITOR
#include "editor/editor_system.h"
#endif // WTIH_EDITOR
#include "ui/utils/fast_lz.h"
#include "event/event_system.h"
#include "data/global.h"
#include "noesis/vulkan_noesis_help.h"
#include <tinyimageformat/tinyimageformat_apis.h>
#define DESCRIPTOR_POOL_MAX_SETS 128
namespace vkn {
	using namespace api;
    constexpr TinyImageFormat CStencilFormat = TinyImageFormat_S8_UINT;
    void vkn::VulkanUISystem::OnBeginRenderFrame(FrameGraph& graph, uint32_t frame)
    {
#ifdef WITH_EDITOR
        graph.mIsRenderEditorSurface = gEngineConfig.IsRenderEditorSurface;
        if (gEngineConfig.IsRenderEditorSurface) {
            graph.mEditorSurfaceID = graph.mSurfaceID;
            graph.mSurfaceID = graph.GetTextureID(FrameGraph::NameEditorSurface, frame);
        }
#endif // WITH_EDITOR
        graph.AddRenderPass<VulkanUISystem>();
    }
    void VulkanUISystem::Setup(FrameGraph& graph, RenderPassBuilder& builder)
    {
#include "windows.h"
        TextureDesc stencil = graph.GetRenderSurface();
        stencil.id = 0;
        stencil.state = ResourceState::UNDEFINED;
        stencil.format = CStencilFormat;
        stencil.image = nullptr;
        stencil.usage = TextureUsage::STENCIL_ATTACHMENT | TextureUsage::DEPTH_ATTACHMENT;
        graph.AcquireTexture(stencil);
        builder.Name(UIPassName)
            .Type(RenderPassNodeType::Imgui, RenderPassNodeFlag::Output)
            .Write(graph.GetRenderSurface(), ResourceState::COLOR_ATTACHMENT)
            .Write(stencil, ResourceState::DEPTH_ATTACHMENT);
        if (gEngineConfig.IsRenderEditorSurface) {
            builder.Read(graph.GetSurface(), ResourceState::READ_ONLY);
        }
    }
    void VulkanUISystem::Execute(FrameGraph& graph, RenderPassContext& context)
    {
        auto& surface = graph.GetRenderSurface();
        context->SetViewport(0.0f, 0.0f, (float)surface.width, (float)surface.height, 0.f, 1.f);
        context->SetScissor(0, 0, surface.width, surface.height);
        graph.GetRenderSurface().state = ResourceState::PRESENT;
#ifdef WITH_EDITOR
        EditorSystem::Ptr()->Render(graph, context);
#endif // WITH_EDITOR
    }
    void VulkanUISystem::InitNoesisRender(bool linearRendering, bool stereoSupport, SampleCount sampleCount_)
    {
        VulkanAPI* API = VulkanAPI::Ptr();
        mDevice = &API->GetBackend().GetDevice();
        TinyImageFormat colorFormat = API->context.surface.format;
        mHasExtendedDynamicState = false;
        mIsLinearRendering = linearRendering;
        VkPhysicalDeviceProperties mDeviceProperties;
        vkGetPhysicalDeviceProperties(mDevice->GetPhysical(), &mDeviceProperties);
        mMinUniformBufferOffsetAlignment = mDeviceProperties.limits.minUniformBufferOffsetAlignment;
        VkSampleCountFlagBits sampleCount = vkApiGetSmpleCountFlag(sampleCount_);
        CreateRenderPass(colorFormat, sampleCount);
        CreateLayouts();
        LoadShaders(stereoSupport);
        CreatePipelines(sampleCount);
        CreateSamplers();
        CreateDescriptorPool();
        EventSystem::Ptr()->BeginRenderFrame.Subscribe(&VulkanUISystem::OnBeginRenderFrame, this);
        API->SetRenderPassInfo(UIPassName, mRenderPass);
    }
    void VulkanUISystem::BeginRender(api::RenderContext* context) {
        VulkanContext* ctx = (VulkanContext*)context;
        mCommandBuffer = ctx->command;
        mFrameNumber = ctx->frameNumber;
        mSafeFrameNumber = ctx->frameNumber - ctx->frameCount;
        mWaitTransferCount = 0;
        mCachedIndexBuffer = nullptr;
        mCachedPipeline = nullptr;
    }
    void VulkanUISystem::DrawBatch(const Noesis::Batch& batch)
    {
        Layout layout = mLayouts[batch.shader.v];
        // Skip draw if shader requires something not available in the batch
        if ((layout.signature & GetSignature(batch)) != layout.signature) 
            return;
        SetBuffers(batch);
        BindDescriptors(batch, layout);
        BindPipeline(batch);
        SetStencilRef(batch.stencilRef);
        auto& mIndices = mRenderBuffer->mIndices;
        uint32_t firstIndex = batch.startIndex + mIndices.drawPos / 2;

        if (batch.singlePassStereo)
        {
#ifdef NS_PLATFORM_ANDROID
            // GL_EXT_multiview
            vkCmdDrawIndexed(mCommandBuffer, batch.numIndices, 1, firstIndex, 0, 0);
#else
            // GL_ARB_shader_viewport_layer_array
            vkCmdDrawIndexed(mCommandBuffer, batch.numIndices, 2, firstIndex, 0, 0);
#endif
        }
        else
        {
            vkCmdDrawIndexed(mCommandBuffer, batch.numIndices, 1, firstIndex, 0, 0);
        }
        if (mWaitTransferCount == 1) {
            mWaitTransferCount += 100;
            VulkanAPI::Ptr()->graph.GetCurrentNode()->SetWaitTransfer();
        }
    }
    void VulkanUISystem::SetBuffers(const Noesis::Batch& batch)
    {
        auto& mVertices = mRenderBuffer->mVertices;
        auto& mIndices = mRenderBuffer->mIndices;
        VkBuffer buffer = (VkBuffer)mVertices.currentPage->pBuffer;
        VkDeviceSize offset = mVertices.drawPos + batch.vertexOffset;
        vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &buffer, &offset);

        if (mCachedIndexBuffer != (VkBuffer)mIndices.currentPage->pBuffer)
        {
            mCachedIndexBuffer = (VkBuffer)mIndices.currentPage->pBuffer;
            vkCmdBindIndexBuffer(mCommandBuffer, mCachedIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
        }
    }
    void VulkanUISystem::BindDescriptors(const Noesis::Batch& batch, const Layout& layout)
    {
        uint32_t signature = layout.signature;

        UITexture* pattern = (UITexture*)batch.pattern;
        UITexture* ramps = (UITexture*)batch.ramps;
        UITexture* image = (UITexture*)batch.image;
        UITexture* glyphs = (UITexture*)batch.glyphs;
        UITexture* shadow = (UITexture*)batch.shadow;

        uint8_t patternSampler = batch.patternSampler.v;
        uint8_t rampsSampler = batch.rampsSampler.v;
        uint8_t imageSampler = batch.imageSampler.v;
        uint8_t glyphsSampler = batch.glyphsSampler.v;
        uint8_t shadowSampler = batch.shadowSampler.v;

        const UniformData* vsCB0 = &batch.vertexUniforms[0];
        const UniformData* vsCB1 = &batch.vertexUniforms[1];
        const UniformData* psCB0 = &batch.pixelUniforms[0];
        const UniformData* psCB1 = &batch.pixelUniforms[1];

        // Although a descriptor pool without VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT should
        // allocate very fast it is recommended to optimize calls to vkAllocateDescriptorSets() for
        // mobiles. The strategy here is hashing each descriptor set for reusing

        uint32_t hash = 0x050c5d1f;
        Vector<uint32_t, 4> offsets;
        if (signature & VS_CB0) UploadUniforms(0, vsCB0, hash, offsets);
        if (signature & VS_CB1) UploadUniforms(1, vsCB1, hash, offsets);
        if (signature & PS_CB0) UploadUniforms(2, psCB0, hash, offsets);
        if (signature & PS_CB1) UploadUniforms(3, psCB1, hash, offsets);

        if (signature & PS_T0) TextureHash(hash, pattern, patternSampler);
        if (signature & PS_T1) TextureHash(hash, ramps, rampsSampler);
        if (signature & PS_T2) TextureHash(hash, image, imageSampler);
        if (signature & PS_T3) TextureHash(hash, glyphs, glyphsSampler);
        if (signature & PS_T4) TextureHash(hash, shadow, shadowSampler);
        auto it = mDescriptorSetMap.Find(hash);

        if (it == mDescriptorSetMap.End())
        {
            if (mDescriptorSetMap.Size() == DESCRIPTOR_POOL_MAX_SETS)
            {
                // Current pools is exhausted, get a new one and clear the cache
                mDescriptorSetMap.Clear();
                CreateDescriptorPool();
            }

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = mDescriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout.setLayout;

            VkDescriptorSet set;
            V(vkAllocateDescriptorSets(mDevice->Ptr(), &allocInfo, &set));

            it = mDescriptorSetMap.Insert(hash, set).first;

            Vector<VkDescriptorBufferInfo, 4> buffers;
            Vector<VkDescriptorImageInfo, 5> images;
            Vector<VkWriteDescriptorSet, 9> writes;

            uint32_t binding = 0;

            if (signature & VS_CB0) FillBufferInfo(0, vsCB0, set, buffers, writes, binding);
            if (signature & VS_CB1) FillBufferInfo(1, vsCB1, set, buffers, writes, binding);
            if (signature & PS_CB0) FillBufferInfo(2, psCB0, set, buffers, writes, binding);
            if (signature & PS_CB1) FillBufferInfo(3, psCB1, set, buffers, writes, binding);

            if (signature & PS_T0) FillImageInfo(pattern, patternSampler, set, images, writes, binding);
            if (signature & PS_T1) FillImageInfo(ramps, rampsSampler, set, images, writes, binding);
            if (signature & PS_T2) FillImageInfo(image, imageSampler, set, images, writes, binding);
            if (signature & PS_T3) FillImageInfo(glyphs, glyphsSampler, set, images, writes, binding);
            if (signature & PS_T4) FillImageInfo(shadow, shadowSampler, set, images, writes, binding);
            vkUpdateDescriptorSets(mDevice->Ptr(), writes.Size(), writes.Data(), 0, nullptr);
        }

        vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout.pipelineLayout,
            0, 1, &it->value, offsets.Size(), offsets.Data());
    }
    void VulkanUISystem::UploadUniforms(uint32_t i, const Noesis::UniformData* data, uint32_t& hash, BaseVector<uint32_t>& offsets)
    {
        auto& mConstants = mRenderBuffer->mConstants;
        if (mCachedConstantHash[i] != data->hash)
        {
            uint32_t size = data->numDwords * sizeof(uint32_t);
            void* ptr = mConstants[i].MapBuffer(meta_align_size(size, mMinUniformBufferOffsetAlignment), mFrameNumber, mSafeFrameNumber);
            memcpy(ptr, data->values, size);

            mCachedConstantHash[i] = data->hash;
        }

        hash = Hash(hash, mConstants[i].currentPage->hash | (data->numDwords << 16));
        offsets.PushBack(mConstants[i].drawPos);
    }
    void VulkanUISystem::TextureHash(uint32_t& hash, UITexture* texture, uint8_t sampler)
    {
        TextureDesc& desc = texture->desc;
        if (desc.id == -1) {
            desc.id = mLastTextureHashValue++;
            texture->imageView = VulkanAPI::Ptr()->CreateTextureView(desc.ToTextureView());
        }
        hash = Hash(hash, desc.id | sampler << 24);
    }
    void VulkanUISystem::FillBufferInfo(uint32_t i, const Noesis::UniformData* data, VkDescriptorSet set, BaseVector<VkDescriptorBufferInfo>& buffers, BaseVector<VkWriteDescriptorSet>& writes, uint32_t& binding)
    {
        auto& mConstants = mRenderBuffer->mConstants;
        VkDescriptorBufferInfo& info = buffers.EmplaceBack();
        info.buffer = (VkBuffer)mConstants[i].currentPage->pBuffer;
        info.range = data->numDwords * sizeof(uint32_t);
        VkWriteDescriptorSet& write = writes.EmplaceBack();
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = binding++;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write.descriptorCount = 1;
        write.pBufferInfo = &info;
    }
    void VulkanUISystem::FillImageInfo(UITexture* texture, uint8_t sampler, VkDescriptorSet set, BaseVector<VkDescriptorImageInfo>& images, BaseVector<VkWriteDescriptorSet>& writes, uint32_t& binding)
    {
        if (texture->desc.state != ResourceState::READ_ONLY) {
            mWaitTransferCount++;
        }
        VkDescriptorImageInfo& info = images.EmplaceBack();
        info.sampler = mSamplers[sampler];
        info.imageView = (VkImageView)texture->imageView;
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet& write = writes.EmplaceBack();
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = binding++;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &info;
    }
    void VulkanUISystem::BindPipeline(const Batch& batch)
    {
        RenderState renderState = batch.renderState;

        if (mHasExtendedDynamicState)
        {
            //SetStencilMode((StencilMode::Enum)renderState.f.stencilMode);
            //renderState.f.stencilMode = 0;
        }
        uint32_t hash = PiplineHashKey{ mRenderPass, batch.shader.v, renderState.v }.hash();
        VkPipeline pipeline = mPipelines[mPipelineMap.Find(hash)->value];

        if (pipeline != mCachedPipeline)
        {
            vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            mCachedPipeline = pipeline;
        }
    }
    void VulkanUISystem::CreateRenderPass(TinyImageFormat format, VkSampleCountFlagBits sampleCount)
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = (VkFormat)TinyImageFormat_ToVkFormat(format);
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription stencilAttachment = {};
        stencilAttachment.format = (VkFormat)TinyImageFormat_ToVkFormat(CStencilFormat);;
        stencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        stencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        stencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        stencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        stencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        stencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        stencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        VkAttachmentDescription attachmentList[2] = { colorAttachment , stencilAttachment };
        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 2;
        info.pAttachments = attachmentList;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;
        V(vkCreateRenderPass(mDevice->Ptr(), &info, VK_NULL_HANDLE, &mRenderPass));
    }
    void VulkanUISystem::CreateLayouts()
    {
        for (uint32_t i = 0; i < Noesis::Shader::Count; i++)
        {
            const ShaderPS& pShader = ShadersPS(i);
            CreateLayout(pShader.signature, mLayouts[i]);
        }
    }
    void VulkanUISystem::CreateLayout(uint32_t signature, Layout& layout)
    {
        uint32_t buffersVS = 0;
        uint32_t buffersPS = 0;
        uint32_t textures = 0;

        if (signature & VS_CB0) buffersVS++;
        if (signature & VS_CB1) buffersVS++;
        if (signature & PS_CB0) buffersPS++;
        if (signature & PS_CB1) buffersPS++;
        if (signature & PS_T0) textures++;
        if (signature & PS_T1) textures++;
        if (signature & PS_T2) textures++;
        if (signature & PS_T3) textures++;
        if (signature & PS_T4) textures++;

        uint32_t hash = buffersVS | (buffersPS << 8) | (textures << 16);
        auto r = mLayoutMap.Insert(hash, layout);

        VkDescriptorSetLayout& setLayout = r.first->value.setLayout;
        VkPipelineLayout& pipelineLayout = r.first->value.pipelineLayout;

        if (r.second)
        {
            // VkDescriptorSetLayout
            Vector<VkDescriptorSetLayoutBinding, 16> layoutBindings;
            FillLayoutBindings(buffersVS, buffersPS, textures, layoutBindings);
            NS_ASSERT(layoutBindings.IsSmall());

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = layoutBindings.Size();
            layoutInfo.pBindings = layoutBindings.Data();

            V(vkCreateDescriptorSetLayout(mDevice->Ptr(), &layoutInfo, nullptr, &setLayout));

            // VkPipelineLayout
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &setLayout;

            V(vkCreatePipelineLayout(mDevice->Ptr(), &pipelineLayoutInfo, nullptr, &pipelineLayout));
        }

        layout.setLayout = setLayout;
        layout.pipelineLayout = pipelineLayout;
        layout.signature = signature;
    }
    void VulkanUISystem::LoadShaders(bool stereoSupport)
	{
		uint8_t* shaders = (uint8_t*)xmalloc(FastLZ::DecompressBufferSize(Shaders));
		FastLZ::Decompress(Shaders, sizeof(Shaders), shaders);
        for (uint32_t i = 0; i < Noesis::Shader::Vertex::Count; i++)
        {
            const ShaderVS& vShader = ShadersVS(i, mIsLinearRendering, stereoSupport);

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = vShader.size;
            createInfo.pCode = (uint32_t*)(shaders + vShader.start);

            V(vkCreateShaderModule(mDevice->Ptr(), &createInfo, nullptr, &mVertexShaders[i]));
        }

        for (uint32_t i = 0; i < Noesis::Shader::Count; i++)
        {
            const ShaderPS& pShader = ShadersPS(i);
            mPixelShaders[i] = VK_NULL_HANDLE;

            if (pShader.label != nullptr)
            {
                VkShaderModuleCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.codeSize = pShader.size;
                createInfo.pCode = (uint32_t*)(shaders + pShader.start);

                V(vkCreateShaderModule(mDevice->Ptr(), &createInfo, nullptr, &mPixelShaders[i]));
            }
        }
        xfree(shaders);
	}
    void VulkanUISystem::CreatePipelines(VkSampleCountFlagBits sampleCount)
    {
        for (uint32_t i = 0; i < Noesis::Shader::Count; i++)
        {
            const ShaderPS& pShader = ShadersPS(i);

            if (pShader.label != nullptr)
            {
                CreatePipelines((uint8_t)i, mPixelShaders[i], mLayouts[i].pipelineLayout, sampleCount);
            }
        }
    }
    void VulkanUISystem::CreatePipelines(uint8_t shader, VkShaderModule psModule, VkPipelineLayout layout, VkSampleCountFlagBits sampleCount)
    {
        uint8_t vsIndex = VertexForShader[shader];

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.renderPass = mRenderPass;
        pipelineInfo.layout = layout;

        // Vertex Input State
        uint32_t format = FormatForVertex[vsIndex];
        Vector<VkVertexInputAttributeDescription, Noesis::Shader::Vertex::Format::Attr::Count> attrs;
        FillVertexAttributes(format, attrs);

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = SizeForFormat[format];
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = attrs.Size();
        vertexInputInfo.pVertexAttributeDescriptions = attrs.Data();

        pipelineInfo.pVertexInputState = &vertexInputInfo;

        // Shader Stages
        VkPipelineShaderStageCreateInfo shaderStages[2] = {};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = mVertexShaders[vsIndex];
        shaderStages[0].pName = "main";

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = psModule;
        shaderStages[1].pName = "main";

        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

        // Multisample State
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = sampleCount;
        multisampling.minSampleShading = 1.0f;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        pipelineInfo.pMultisampleState = &multisampling;

        // Dynamic State
        Vector<VkDynamicState, 16> dynamicStates;
        dynamicStates.PushBack(VK_DYNAMIC_STATE_SCISSOR);
        dynamicStates.PushBack(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStates.PushBack(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

        if (mHasExtendedDynamicState)
        {
            dynamicStates.PushBack(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT);
            dynamicStates.PushBack(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT);
            dynamicStates.PushBack(VK_DYNAMIC_STATE_STENCIL_OP_EXT);

            // There appears to be a driver bug in the Meta Quest 2 (Adreno 650) where the compare
            // and write mask must be set as dynamic, even though their values remain constant
            dynamicStates.PushBack(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
            dynamicStates.PushBack(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
        }

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = dynamicStates.Size();
        dynamicState.pDynamicStates = dynamicStates.Data();

        pipelineInfo.pDynamicState = &dynamicState;

        // Viewport State
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        pipelineInfo.pViewportState = &viewportState;

        // Input Assembly State
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        pipelineInfo.pInputAssemblyState = &inputAssembly;

        CreatePipelines(shader, pipelineInfo);
    }
    void VulkanUISystem::CreatePipelines(uint8_t shader_, VkGraphicsPipelineCreateInfo& pipelineInfo)
    {
        VkPipeline parent = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures mDeviceFeatures;
        vkGetPhysicalDeviceFeatures(mDevice->GetPhysical(), &mDeviceFeatures);
        for (uint32_t i = 0; i < 256; i++)
        {
            Noesis::Shader shader;
            shader.v = shader_;

            RenderState state;
            state.v = (uint8_t)i;

            if (!IsValidShader(shader, state, mHasExtendedDynamicState)) {
                continue;
            }
            pipelineInfo.basePipelineHandle = parent;
            pipelineInfo.flags = (parent != VK_NULL_HANDLE) ? VK_PIPELINE_CREATE_DERIVATIVE_BIT :
                VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
            VkPipelineRasterizationStateCreateInfo rasterizer{};
            RasterizerInfo(rasterizer, state, mDeviceFeatures);

            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            DepthStencilInfo(depthStencil, state);

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            BlendInfo(colorBlendAttachment, state);

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;

            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pDepthStencilState = &depthStencil;
            pipelineInfo.pColorBlendState = &colorBlending;

            VkPipeline pipeline;
            V(vkCreateGraphicsPipelines(mDevice->Ptr(), mPipelineCache, 1, &pipelineInfo, 0, &pipeline));

            parent = (parent == VK_NULL_HANDLE) ? pipeline : parent;
            uint32_t hash = PiplineHashKey{ mRenderPass, shader_, state.v }.hash();
            PiplineHashKey res{ mRenderPass, shader_, state.v };
            mPipelineMap.Insert(hash, mPipelines.Size());
            mPipelines.PushBack(pipeline);
        }
    }
    void VulkanUISystem::CreateSamplers()
    {
        memset(mSamplers, 0, sizeof(mSamplers));

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        samplerInfo.mipLodBias = -0.75f;

        const char* MinMagStr[] = { "Nearest", "Linear" };
        const char* MipStr[] = { "Disabled", "Nearest", "Linear" };
        const char* WrapStr[] = { "ClampToEdge", "ClampToZero", "Repeat", "MirrorU", "MirrorV", "Mirror" };

        for (uint8_t minmag = 0; minmag < MinMagFilter::Count; minmag++)
        {
            for (uint8_t mip = 0; mip < MipFilter::Count; mip++)
            {
                SetMinMagFilter(MinMagFilter::Enum(minmag), samplerInfo);
                SetMipFilter(MipFilter::Enum(mip), samplerInfo);

                for (uint8_t uv = 0; uv < WrapMode::Count; uv++)
                {
                    SetAddress(WrapMode::Enum(uv), samplerInfo);

                    SamplerState s = { { uv, minmag, mip } };
                    V(vkCreateSampler(mDevice->Ptr(), &samplerInfo, nullptr, &mSamplers[s.v]));
                }
            }
        }
    }
    void VulkanUISystem::SetStencilRef(uint32_t stencilRef)
    {
        if (mCachedStencilRef != stencilRef)
        {
            vkCmdSetStencilReference(mCommandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, stencilRef);
            mCachedStencilRef = stencilRef;
        }
    }
    void VulkanUISystem::CreateDescriptorPool()
    {
        if (mDescriptorPool)
        {
            // Current pool is exhausted
            mFreeDescriptorPools.PushBack(Pair(mDescriptorPool, mFrameNumber));

            // If possible, recycle a previously created pool
            if (mFreeDescriptorPools.Size() > 1)
            {
                if (mFreeDescriptorPools.Front().second <= mSafeFrameNumber)
                {
                    mDescriptorPool = mFreeDescriptorPools.Front().first;
                    V(vkResetDescriptorPool(mDevice->Ptr(), mDescriptorPool, 0));
                    mFreeDescriptorPools.Erase(mFreeDescriptorPools.Begin());
                    return;
                }
            }
        }

        VkDescriptorPoolSize poolSizes[2] =
        {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTOR_POOL_MAX_SETS * 4 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTOR_POOL_MAX_SETS * 3 }
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 2;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = DESCRIPTOR_POOL_MAX_SETS;

        V(vkCreateDescriptorPool(mDevice->Ptr(), &poolInfo, nullptr, &mDescriptorPool));
    }
}
