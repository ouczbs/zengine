#include "vkn/vulkan_api.h"
#include "vkn/vulkan_window.h"
#include "vkn/vulkan_api_help.h"
#include "vkn/wrapper/device.h"
#include "vkn/thread/buffer_worker.h"
#include "vkn/thread/command_worker.h"
#include "vkn/loader/vulkan_glsl_loader.h"
#include "render/asset/mesh.h"
#include "event/event_system.h"
#include "tinyimageformat/tinyimageformat_apis.h"
#include "zlog.h"
namespace vkn {
	using api::EventSystem;
	VulkanAPI::VulkanAPI(VulkanWindow* pWindow) : RenderAPI(new VulkanContext())
		, window(*pWindow)
		, backend(VulkanEngineName) 
	{

	}
	void VulkanAPI::Init()
	{
		Backend::TransferWorker->InitCommandBuffers(context.frameCount);
	}
	void VulkanAPI::Shutdown()
	{

	}
	void VulkanAPI::SetStaticMesh(Mesh& mesh)
	{
		auto& Indices = mesh.GetIndices();
		auto& Vertices = mesh.GetVertices();
		MeshVAO& VAO = MeshTable[mesh.GetGuid()];
		VAO.indexCount = Indices.size();
		VAO.vertexCount = Vertices.size();

		BufferUpload indexBuffer{};
		indexBuffer.pBuffer = &VAO.indexBuffer;
		indexBuffer.pAllocation = &VAO.indexAllocation;
		indexBuffer.pCpuData = Indices.data();
		indexBuffer.size = sizeof(decltype(Indices[0])) * Indices.size();
		indexBuffer.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		Backend::TransferWorker->Invoke(indexBuffer);

		BufferUpload vertexBuffer{};
		vertexBuffer.pBuffer = &VAO.vertexBuffer;
		vertexBuffer.pAllocation = &VAO.vertexAllocation;
		vertexBuffer.pCpuData = Vertices.data();
		vertexBuffer.size = Vertices.data_size();
		vertexBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		Backend::TransferWorker->Invoke(vertexBuffer);
	}
	void VulkanAPI::DrawStaticMesh(Mesh& mesh)
	{
		MeshVAO& vulkanVAO = MeshTable[mesh.GetGuid()];
		if (!vulkanVAO.indexBuffer || !vulkanVAO.vertexBuffer) {
			return;
		}
		VulkanPipeline& pipeline = PipelineTable[mesh.GetShaderGuid()];
		VulkanContext& ctx = *(VulkanContext*)&context;
		VkCommandBuffer ptr = ctx.command;
		VkBuffer vertexBuffers[] = { vulkanVAO.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(ptr, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(ptr, vulkanVAO.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindPipeline(ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
		if (pipeline.descCount > 0) {
			auto& materialInst = mesh.GetMaterialInstance();
			auto& materialInfo = materialInst.GetInfo();
			auto& gpuBlock = materialInfo.gpuBlock;
			uint32_t staticCount = materialInfo.staticBlock.count;
			materialInfo.staticBlock.Upload(gpuBlock.pMappingAddr);
			materialInfo.classBlock.Upload(gpuBlock.pMappingAddr + staticCount);
			vkCmdBindDescriptorSets(ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, pipeline.descCount, pipeline.descList, 0, VK_NULL_HANDLE);
		}
		vkCmdDrawIndexed(ptr, vulkanVAO.indexCount, 1, 0, 0, 0);
	}
	void VulkanAPI::SetUpMaterialInstance(MaterialInstance& material)
	{
		Shader& shader = *material->GetShader();
		VulkanPipeline& pipeline = PipelineTable[shader.GetGuid()];
		pmr::vector<api::ShaderProgram*> programList{ FramePool() };
		programList.push_back(shader.GetVertHandle().Ptr());
		programList.push_back(shader.GetFragHandle().Ptr());
		VulkanGlslLoader::LoadShaderBuffer(pipeline.descList, programList, material.GetInfo());
	}
	void VulkanAPI::LoadShader(Shader& shader, size_t passKey)
	{
		auto itPass = RenderPassCache.find(passKey);
		if (itPass == RenderPassCache.end()) {
			return;
		}
		VkRenderPass renderpass = itPass->second.pass;
		pmr::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::map<VkShaderStageFlagBits, VkShaderModule> shaderModules;
		auto& device = backend.GetDevice();
		pmr::vector<api::ShaderProgram*> programList{FramePool()};
		programList.push_back(shader.GetVertHandle().Ptr());
		programList.push_back(shader.GetFragHandle().Ptr());
		for (auto program : programList) {
			vkShaderProgram* vkProgram = (vkShaderProgram*)program;
			auto shaderModule = vkProgram->Ptr();
			shaderModules.insert(std::make_pair(vkProgram->GetVkStage(), shaderModule));
		}
		for (auto& shaderModule : shaderModules)
		{
			VkPipelineShaderStageCreateInfo shaderStageInfo = {};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = shaderModule.first;
			shaderStageInfo.module = shaderModule.second;
			shaderStageInfo.pName = "main";
			shaderStages.push_back(shaderStageInfo);
		}
		auto meta = refl::find_meta(shader.Name(),string_hash("vkMeta"));
		// 设置顶点输入格式
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = meta->size;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		//这里顶点属性不能大余16
		std::array<VkVertexInputAttributeDescription, 16> attributeDescriptions = { };
		{
			uint32_t count = 0;
			for (auto& field : meta->GetFields(refl::FIND_ALL_MEMBER, Name(""))) {
				auto& attr = attributeDescriptions[count];
				attr.binding = 0;
				attr.location = count++;
				attr.format = field.GetMeta() ? (VkFormat)field.GetMeta().CastTo<uint32_t>() : VK_FORMAT_R32G32B32_SFLOAT;
				attr.offset = field.GetOffset();
			}
			vertexInputInfo.vertexAttributeDescriptionCount = count;
		}
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.vertexBindingDescriptionCount = 1;

		// 设置图元
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// ViewPort信息，这里不直接设置，下面弄成动态的
		VkPipelineViewportStateCreateInfo viewportStateInfo = {};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.scissorCount = 1;

		// View Port和Scissor设置为动态，每帧绘制时决定
		pmr::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.pDynamicStates = dynamicStates.data();
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

		// 设置光栅化阶段
		VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// 如果depthClampEnable设置为VK_TRUE，超过远近裁剪面的片元会进行收敛，而不是丢弃它们
		rasterizationInfo.depthClampEnable = VK_FALSE;
		// 如果rasterizerDiscardEnable设置为VK_TRUE，那么几何图元永远不会传递到光栅化阶段
		// 这是禁止任何数据输出到framebuffer的方法
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		// 设置片元如何从几何模型中产生，如果不是FILL，需要开启GPU feature
		// VK_POLYGON_MODE_FILL: 多边形区域填充
		// VK_POLYGON_MODE_LINE: 多边形边缘线框绘制
		// VK_POLYGON_MODE_POINT : 多边形顶点作为描点绘制
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.lineWidth = 1.0f;
		rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		// 渲染阴影的偏移配置
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthBiasConstantFactor = 0.0f;
		rasterizationInfo.depthBiasClamp = 0.0f;
		rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		// 设置Shader采样纹理的MSAA(不是输出到屏幕上的MSAA)，需要创建逻辑设备的时候开启VkPhysicalDeviceFeatures里的sampleRateShading才能生效，暂时关闭
		VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.sampleShadingEnable = (VK_SAMPLE_COUNT_1_BIT & VK_SAMPLE_COUNT_1_BIT ? VK_FALSE : VK_TRUE);
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		// 这个是调整sampleShading效果的，越接近1效果越平滑，越接近0性能越好
		multisampleInfo.minSampleShading = 1.0f;
		multisampleInfo.pSampleMask = VK_NULL_HANDLE;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;
		// Color Blend
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.attachmentCount = 1;
		// 深度和模板配置
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		// Depth
		depthStencilInfo.depthWriteEnable = VK_FALSE;
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;
		// Stencil
		depthStencilInfo.stencilTestEnable = VK_FALSE;
		depthStencilInfo.front = {};
		depthStencilInfo.back = {};

		VulkanPipeline& vulkan_pipeline = PipelineTable[shader.GetGuid()];
		vulkan_pipeline.descCount = VulkanGlslLoader::GetShaderLayout(vulkan_pipeline.descLayoutList, programList);
		VkDescriptorSet descriptorSet;
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = backend.GetPool().Ptr();
		allocInfo.descriptorSetCount = vulkan_pipeline.descCount;
		allocInfo.pSetLayouts = vulkan_pipeline.descLayoutList;
		vkAllocateDescriptorSets(device.Ptr(), &allocInfo, vulkan_pipeline.descList);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = vulkan_pipeline.descCount;
		pipelineLayoutInfo.pSetLayouts = vulkan_pipeline.descLayoutList;
		VkPipelineLayout pipelineLayout;
		if (vkCreatePipelineLayout(device.Ptr(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = nullptr; //&depthStencilInfo;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderpass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VkPipeline pipeLine;
		if (vkCreateGraphicsPipelines(device.Ptr(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeLine) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics pipeline!");

		for (auto& shaderModule : shaderModules)
			vkDestroyShaderModule(device.Ptr(), shaderModule.second, nullptr);

		vulkan_pipeline.name = shader.Name();
		vulkan_pipeline.pipeline = pipeLine;
		vulkan_pipeline.inUse = true;
		vulkan_pipeline.pipelineLayout = pipelineLayout;
	}
	void VulkanAPI::CreateBuffer(BufferDesc& desc)
	{
		BufferCreator creator{};
		creator.size = desc.size;
		creator.pBuffer = (VkBuffer*)&desc.buffer;
		creator.pAllocation =(VmaAllocation*) &desc.pAllocation;
		creator.ppCpuData = &desc.pMappingAddr;
		creator.memoryUsage = vkApiGetMemoryUsage(desc.memoryUsage);
		creator.momoryFlags = vkApiGetMemoryFlags(desc.memoryUsage);
		creator.usage = vkApiGetBufferUsage(desc.usage);
		Backend::TransferWorker->CreateBuffer(creator);
	}
	void VulkanAPI::CreateTexture(TextureDesc& desc)
	{
		VkImageCreateInfo imageCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = vkApiGetImageCreateFlag(desc.dimension, desc.arraySize),
			.imageType = vkApiGetImageType(desc.dimension),
			.format = (VkFormat)TinyImageFormat_ToVkFormat(desc.format),
			.extent = {
				.width = (uint32_t)desc.width,
				.height = (uint32_t)desc.height,
				.depth = (uint32_t)desc.depth,
				},
			.mipLevels = desc.mipLevel,
			.arrayLayers = desc.arraySize,
			.samples = vkApiGetSmpleCountFlag(desc.sampleCount),
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = vkApiGetImageUsageFlags(desc.usage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = NULL,
			.initialLayout = vkApiGetImageLayout(desc.state)
		};
		ImageCreator imageCreator{ .imageInfo = imageCreateInfo ,.image = (VkImage*)&desc.image };
		Backend::TransferWorker->CreateImage(imageCreator);
	}
	ImageViewPtr VulkanAPI::CreateTextureView(TextureViewKey desc)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = (VkImage)desc.image;
		createInfo.viewType = vkApiGetImageViewType(desc.dimension, desc.layerCount);
		createInfo.format = (VkFormat)TinyImageFormat_ToVkFormat(desc.format);

		// components字段允许调整颜色通道的最终的映射逻辑
		// 比如，我们可以将所有颜色通道映射为红色通道，以实现单色纹理，我们也可以将通道映射具体的常量数值0和1
		// 这里用默认的
		createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_A;

		// subresourceRangle字段用于描述图像的使用目标是什么，以及可以被访问的有效区域
		// 这个图像用作填充color还是depth stencil等
		createInfo.subresourceRange.aspectMask = vkApiGetImageAspectMask(createInfo.format, false);
		// 默认处理所有Mipmap
		createInfo.subresourceRange.baseMipLevel = desc.baseMipLevel;
		createInfo.subresourceRange.levelCount = desc.levelCount;
		// 默认处理所有Layers
		createInfo.subresourceRange.baseArrayLayer = desc.baseArrayLayer;
		createInfo.subresourceRange.layerCount = desc.layerCount;

		VkImageView imageView;
		vkCreateImageView(backend.GetDevice().Ptr(), &createInfo, nullptr, &imageView);
		return (ImageViewPtr)imageView;
	}
	SamplerPtr VulkanAPI::CreateTextureSampler(TextureSampler key)
	{
		VkSamplerCreateInfo samplerInfo = vkApiGetSamplerCreateInfo(key);
		VkSampler sampler;
		vkCreateSampler(backend.GetDevice().Ptr(), &samplerInfo, nullptr, &sampler);
		return sampler;
	}
	void VulkanAPI::BeginFrame()
	{
		VulkanContext& ctx = *(VulkanContext*)&context;
		window.Aquire(ctx);
		graph.Input(ctx.surface);
		EventSystem::Ptr()->BeginRenderFrame.Invoke(graph, ctx.frame);
	}
	void VulkanAPI::EndFrame()
	{
		VulkanContext& ctx = *(VulkanContext*)&context;
		ctx.FlushCommand();
		window.Present(ctx);
		Backend::TransferWorker->TrySyncTransfer(ctx);
	}
	void VulkanAPI::BeginRenderPass(RenderPassNode* node, FnEnterRenderPass callback)
	{
		RenderPassKey     config{};
		FramebufferKey    frameKey{.layers = 1};
		VkClearValue      clearValues[MAX_SUPPORTED_RENDER_TARGET_COUNT] = { 0 };
		RenderPassParams& params = node->params;
		int clearSurfaceIndex = -1;
		int i = 0, attachmentCount = 0;
		for (auto& it : node->outEdges) {
			uint32_t flag = 1 << i;
			TextureDesc& texture = it->CastTo<TextureDesc>();
			frameKey.imageViews[attachmentCount++] = (VkImageView)graph.ResolveTextureView(texture);
			frameKey.height = texture.height;
			frameKey.width = texture.width;
			config.colorFormat[i] = texture.format;
			if (params.sampleMask & flag) {
				TextureDesc resolve = texture.Resolve();
				graph.AcquireTexture(resolve);
				frameKey.imageViews[attachmentCount++] = (VkImageView)graph.ResolveTextureView(resolve);
			}
			if (texture.sampleCount > config.samples)
				config.samples = texture.sampleCount;
			VkClearValue& clearValue = clearValues[i];
			const bool bclear = params.clear & flag;
			if (texture.state == ResourceState::COLOR_ATTACHMENT) {
				if (!bclear && context.surface.id == texture.id && node->IsFirstInput()) {
					clearSurfaceIndex = i;//需要手动清除
				}
				if (bclear || clearSurfaceIndex == i) {
					clearValue.color.float32[0] = params.clearColor.r;
					clearValue.color.float32[1] = params.clearColor.g;
					clearValue.color.float32[2] = params.clearColor.b;
					clearValue.color.float32[3] = params.clearColor.a;
				}
			}
			else {
				config.depthMask |= flag;
				if (bclear) {
					clearValue.depthStencil = { (float)params.clearDepth, params.clearStencil };
				}
			}
			i++;
		}
		frameKey.attachmentCount = attachmentCount;
		config.clear = params.clear;
		config.discardEnd = params.discardEnd;
		config.discardStart = params.discardStart;
		RenderPassInfo* passInfo = GetRenderPassInfo(node->name, node->hash);
		if (!passInfo) {
			passInfo = GetRenderPassInfo(node->hash, config);
		}
		frameKey.pass = passInfo->pass;
		auto it = FramebufferCache.find(frameKey);
		VkFramebuffer framebuffer = it->second;
		if (it == FramebufferCache.end()) {
			VkFramebufferCreateInfo framebufferInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = frameKey.pass,
				.attachmentCount = frameKey.attachmentCount,
				.pAttachments = frameKey.imageViews,
				.width = frameKey.width,
				.height = frameKey.height,
				.layers = frameKey.layers
			};
			vkCreateFramebuffer(backend.GetDevice().Ptr(), &framebufferInfo, nullptr, &framebuffer);
			FramebufferCache.emplace(frameKey, framebuffer);
		}
		VkRect2D renderAarea = {
			.offset = {0,0},
			.extent = {frameKey.width,frameKey.height}
		};
		VkRenderPassBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = VK_NULL_HANDLE,
			.renderPass = frameKey.pass,
			.framebuffer = framebuffer,
			.renderArea = renderAarea,
			.clearValueCount = frameKey.attachmentCount,
			.pClearValues = clearValues
		};
		VulkanContext& ctx = *(VulkanContext*)&context;
		CommandBuffer cmd = passInfo->commands[context.frame];
		ctx.command = cmd.Ptr();
		cmd.BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		ctx.renderPassState = RenderPassState::BeginRecord;
		if(callback) callback(node);
		vkCmdBeginRenderPass(cmd.Ptr(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		ctx.renderPassState = RenderPassState::BeginRender;
		if (clearSurfaceIndex != -1)ctx.ClearSurface(clearValues[clearSurfaceIndex].color);
	}
	void VulkanAPI::EndRenderPass(RenderPassNode* node)
	{
		VulkanContext& ctx = *(VulkanContext*)&context;
		RenderPassInfo* passInfo = GetRenderPassInfo(node->name, node->hash);
		CommandBuffer cmd = passInfo->commands[context.frame];
		vkCmdEndRenderPass(cmd.Ptr());
		cmd.EndRecord();
		ctx.renderPassState = RenderPassState::Default;
		VkSemaphore waitSemaphores[8];
		VkPipelineStageFlags waitDstStageMasks[8];
		uint32_t    semaphoreCount = 0;
		if (node->IsFirstInput()) {
			waitDstStageMasks[semaphoreCount] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			waitSemaphores[semaphoreCount++] = ctx.surfaceSemaphore;
			gLogSemaphore("-----wait {:#x}", (uintptr_t)ctx.surfaceSemaphore);
		}
		if (node->IsWaitTransfer()) {
			ctx.FlushCommand();
			waitDstStageMasks[semaphoreCount] = VK_PIPELINE_STAGE_TRANSFER_BIT;
			waitSemaphores[semaphoreCount++] = ctx.transferSemaphore;
			gLogSemaphore("-----wait {:#x}", (uintptr_t)ctx.transferSemaphore);
		}
		for (auto& it : node->dependencies) {
			RenderPassInfo* inputInfo = GetRenderPassInfo(it->name ,it->hash);
			waitDstStageMasks[semaphoreCount] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			waitSemaphores[semaphoreCount++] = inputInfo->semaphores[context.frame];
			gLogSemaphore("-----wait {:#x}", (uintptr_t)inputInfo->semaphores[context.frame]);
		}
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd.Ptr();
		submitInfo.pSignalSemaphores = &passInfo->semaphores[context.frame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitDstStageMasks;
		submitInfo.waitSemaphoreCount = semaphoreCount;
		VkFence fence = nullptr;
		if (node->IsLastOutput()) {
			ctx.graphSemaphore = passInfo->semaphores[context.frame];
			TextureDesc& surface = graph.GetRenderSurface();
			if (surface.state == ResourceState::PRESENT) {
				fence = ctx.surfaceFence;
			}
		}
		gLogSemaphore("+++++sign {:#x}", (uintptr_t)passInfo->semaphores[context.frame]);
		vkQueueSubmit(Backend::RenderWorker->GetQueue().Ptr(), 1, &submitInfo, fence);
	}
	RenderPassInfo* VulkanAPI::GetRenderPassInfo(Name name, size_t hash) {
		if (hash) {
			auto it = RenderPassCache.find(hash);
			if (it != RenderPassCache.end()) {
				return &it->second;
			}
		}
		if (name) {
			auto it = RenderPassNameCache.find(name);
			if (it != RenderPassNameCache.end()) {
				return &it->second;
			}
		}
		return nullptr;
	}
	//单一renderpass，config.passMask 可以取 0
	RenderPassInfo* VulkanAPI::GetRenderPassInfo(size_t& hash, const RenderPassKey& config) {
		hash = config;
		if (auto it = RenderPassCache.find(hash); it != RenderPassCache.end()) {
			return &it->second;
		}
		// Set up some const aliases for terseness.
		const VkAttachmentLoadOp kClear = VK_ATTACHMENT_LOAD_OP_CLEAR;
		const VkAttachmentLoadOp kDontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		const VkAttachmentLoadOp kKeep = VK_ATTACHMENT_LOAD_OP_LOAD;
		const VkAttachmentStoreOp kDisableStore = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		const VkAttachmentStoreOp kEnableStore = VK_ATTACHMENT_STORE_OP_STORE;

		VkAttachmentReference inputAttachmentRef[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
		VkAttachmentReference colorAttachmentRefs[2][MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
		VkAttachmentReference resolveAttachmentRef[2][MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
		VkAttachmentReference depthAttachmentRef[2] = {};

		const bool hasSubpasses = config.subpassMask;
		const bool hasDepth1 = (!hasSubpasses && config.depthMask) || config.depthMask & config.passMask;
		const bool hasDepth2 = config.depthMask & config.subpassMask;
		const bool hasSample1 = (!hasSubpasses && config.sampleMask) || config.sampleMask & config.passMask;
		const bool hasSample2 = config.sampleMask & config.subpassMask;
		VkSubpassDescription subpasses[2] = { {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.pInputAttachments = nullptr,
			.pColorAttachments = colorAttachmentRefs[0],
			.pResolveAttachments = hasSample1 ? resolveAttachmentRef[0] : nullptr,
			.pDepthStencilAttachment = hasDepth1 ? &depthAttachmentRef[0] : nullptr
		},
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.pInputAttachments = inputAttachmentRef,
			.pColorAttachments = colorAttachmentRefs[1],
			.pResolveAttachments = hasSample2 ? resolveAttachmentRef[1] : nullptr,
			.pDepthStencilAttachment = hasDepth2 ? &depthAttachmentRef[1] : nullptr
		} };
	
		VkAttachmentDescription attachments[MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
		// We support 2 subpasses, which means we need to supply 1 dependency struct.
		VkSubpassDependency dependencies[1] = { {
			.srcSubpass = 0,
			.dstSubpass = 1,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
		} };
		// Finally, create the VkRenderPass.
		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 0u,
			.pAttachments = attachments,
			.subpassCount = hasSubpasses ? 2u : 1u,
			.pSubpasses = subpasses,
			.dependencyCount = hasSubpasses ? 1u : 0u,
		};
		const VkSampleCountFlagBits samplecount = vkApiGetSmpleCountFlag(config.samples);
		uint32_t attachmentIndex = 0, samplePassIndex1 = 0, samplePassIndex2 = 0;
		// Populate the Color Attachments.
		for (uint32_t i = 0; i < MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
			if (config.colorFormat[i] == TinyImageFormat_UNDEFINED) {
				break;
			}
			const uint8_t flag = 1 << i;
			const bool clear = config.clear & flag;
			const bool discard = config.discardStart & flag;
			const bool discardEnd = config.discardEnd & flag;
			const bool sample = config.sampleMask & flag;
			const VkFormat format = (VkFormat)TinyImageFormat_ToVkFormat(config.colorFormat[i]);
			VkImageLayout layout = vkApiGetAttachmentLayout(format, true);
			attachments[attachmentIndex] = {
				.format = format,
				.samples = sample ? samplecount : VK_SAMPLE_COUNT_1_BIT,
				.loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
				.storeOp = discardEnd ? kDisableStore : kEnableStore,
				.stencilLoadOp = kDontCare,
				.stencilStoreOp = kDisableStore,
				.initialLayout = layout,
				.finalLayout = layout,
			};
			if (sample) {
				attachments[attachmentIndex + 1] = {
					.format = format,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
					.storeOp = discardEnd ? kDisableStore : kEnableStore,
					.stencilLoadOp = kDontCare,
					.stencilStoreOp = kDisableStore,
					.initialLayout = layout,
					.finalLayout = layout,
				};
			}
			const bool isDepth = config.depthMask & flag;
			const bool isMask1 = !hasSubpasses || config.passMask & flag;
			const bool isMask2 = hasSubpasses && config.subpassMask & flag;
			uint32_t index = 0;
			if (isMask1) {
				index = subpasses[0].colorAttachmentCount++;
				colorAttachmentRefs[0][index].layout = layout;
				colorAttachmentRefs[0][index].attachment = attachmentIndex;
				if (sample) {
					index = subpasses[0].colorAttachmentCount++;
					colorAttachmentRefs[0][index].layout = layout;
					colorAttachmentRefs[0][index].attachment = attachmentIndex + 1;
				}
			}
			if (isMask2) {
				if (isMask1) {
					index = subpasses[1].inputAttachmentCount++;
					inputAttachmentRef[index].layout = layout;
					inputAttachmentRef[index].attachment = sample ? attachmentIndex + 1 : attachmentIndex;
					if (isDepth) {
						dependencies->srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
						dependencies->dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						dependencies->srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						dependencies->dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					}
					else {
						dependencies->srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						dependencies->dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dependencies->srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						dependencies->dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
					}
				}
				else {
					index = subpasses[1].colorAttachmentCount++;
					colorAttachmentRefs[1][index].layout = layout;
					colorAttachmentRefs[1][index].attachment = attachmentIndex;
					if (sample) {
						index = subpasses[1].colorAttachmentCount++;
						colorAttachmentRefs[1][index].layout = layout;
						colorAttachmentRefs[1][index].attachment = attachmentIndex + 1;
					}
				}
			}
			if (isDepth) {
				if (isMask1) {
					depthAttachmentRef[0].layout = layout;
					depthAttachmentRef[0].attachment = attachmentIndex;
				}
				if (isMask2) {
					depthAttachmentRef[1].layout = layout;
					depthAttachmentRef[1].attachment = attachmentIndex;
				}
			}
			attachmentIndex += sample ?  2 : 1;
		}
		renderPassInfo.pDependencies = dependencies->srcStageMask ? dependencies : nullptr;
		renderPassInfo.attachmentCount = attachmentIndex;
		VkRenderPass pass;
		VkResult error = vkCreateRenderPass(backend.GetDevice().Ptr(), &renderPassInfo, nullptr, &pass);
		RenderPassInfo info{pass, config};
		backend.GetDevice().CreateSemaphores(info.semaphores, context.frameCount);
		Backend::RenderWorker->GetCommandPool().PopList(info.commands, context.frameCount);
		auto itr = RenderPassCache.emplace(hash, info);
		return &itr.first->second;
	}
	void VulkanAPI::SetRenderPassInfo(Name name, VkRenderPass pass) {
		RenderPassInfo info{pass};
		backend.GetDevice().CreateSemaphores(info.semaphores, context.frameCount);
		Backend::RenderWorker->GetCommandPool().PopList(info.commands, context.frameCount);
		RenderPassNameCache.emplace(name, info);
	}
}
