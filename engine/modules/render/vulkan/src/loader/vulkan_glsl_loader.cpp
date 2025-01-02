#include "vkn/loader/vulkan_glsl_loader.h"
#include "vkn/vulkan_api.h"
#include "vkn/wrapper/device.h"
#include "vkn/vulkan_api_help.h"
#include "vkn/thread/buffer_worker.h"
#include "render/asset/material.h"
#include "render/tool/glsl_to_spirv.h"
#include "render/asset/vertex.h"
#include "render/asset/ubo.h"
#include "os/file_handle.h"
#include ".render/vkmeta_vertex_gen.inl"
using namespace api;
namespace vkn {
	table<Name, MaterialInfo> MaterialInfoTable;
	void VulkanGlslLoader::Init()
	{
		refl::register_meta<PosVertex>();
		refl::register_meta<TexVertex>();
		refl::register_meta<BoneVertex>();
		ResourceSystem::Ptr()->RegisterLoader<VulkanGlslLoader>(".geom");
		ResourceSystem::Ptr()->RegisterLoader<VulkanGlslLoader>(".frag");
		ResourceSystem::Ptr()->RegisterLoader<VulkanGlslLoader>(".vert");
	}
	ResourceBundle VulkanGlslLoader::LoadFile(PackagePath path, const MetaBundle& meta)
	{
		auto m = meta.FetchMeta<ShaderProgram>();
		auto shader_kind = GlslToSpirv::GetShaderKind(path.GetExtension().ToStringView());
		auto shader_stage = GlslToSpirv::GetShaderStage(shader_kind);
		auto program = m ? ResourceSystem::Ptr()->LoadEmplaceResource<vkShaderProgram>(m->guid, shader_stage)
			: ResourceSystem::Ptr()->LoadEmplaceResource<vkShaderProgram>(shader_stage);
		if (m && m->meta) {
			m->meta.MoveTo(program.Ptr());
		}
		FileHandle handle(path);
		if (!handle.Open(FILE_OP::READ, mFileFlag & FileFlag::File_Binary)) {
			return program;
		}
		if (mFileFlag & FileFlag::File_Binary) {
			pmr::vector<char> data = handle.ReadAll();
			pmr::vector<uint32_t> spirv(data.size() / 4);
			std::memcpy(spirv.data(), data.data(), data.size());
			program->Load(spirv);
		}
		else {
			pmr::string glsl = handle.ReadAll<pmr::string>();
			auto spirv = GlslToSpirv::ToSpirv(glsl, shader_kind, path.GetFileName());
			if (spirv) {
				program->Load(*spirv);
			}
		}
		return program;
	}
	VkDescriptorSetLayout CreateShaderLayout(std::span<ShaderDescriptorLayout> descList, pmr::vector<VkDescriptorSetLayoutBinding>& bindingList) {
		VkDescriptorSetLayout layout;
		bindingList.resize(descList.size());
		uint32_t i = 0;
		for (auto& desc : descList) {
			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = desc.binding;
			binding.descriptorType = vkApiGetDescriptorType(desc.type);
			binding.descriptorCount = 1;
			binding.stageFlags = vkApiGetShaderStageFlags(desc.stageFlags);
			bindingList[i++] = binding;
		}
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pBindings = bindingList.data();
		layoutInfo.bindingCount = static_cast<uint32_t>(bindingList.size());
		vkCreateDescriptorSetLayout(VulkanAPI::Ptr()->GetBackend().GetDevice().Ptr(), &layoutInfo, nullptr, &layout);
		return layout;
	}
	uint32_t VulkanGlslLoader::GetShaderLayout(VkDescriptorSetLayoutList& layoutList, pmr::vector<api::ShaderProgram*>& programList)
	{
		ShaderDescriptorSet descList{FramePool()};
		GlslToSpirv::GetShaderLayout(descList, programList);
		if (descList.empty()) {
			return 0;
		}
		descList.push_back(ShaderDescriptorLayout{.set = 0xff});
		pmr::vector<VkDescriptorSetLayoutBinding> bindingList{FramePool()};
		uint32_t set = 0, binding = 0, count = 0;
		for (auto it = descList.begin(), itEnd = descList.end(); it != itEnd; it++) {
			if (set == it->set) {
				binding++;
			}
			else {
				if (binding > 0) {
					assert(count < MAX_SHADER_DESCRIPTOR_SET_COUNT);
					std::span<ShaderDescriptorLayout> spanList{ it - binding, binding };
					layoutList[count++] = CreateShaderLayout(spanList, bindingList);
				}
				set = it->set;
				binding = 0;
			}
		}
		return count;
	}
	void InitMaterialGpuResourceBlock(MaterialGpuResourceBlock& block,  std::span<const refl::FieldPtr> fields) {
		block.isInit = true;
		uint32_t count = fields.size();
		block.bufferOffset = meta_align_size(sizeof(void*) * count);
		uint32_t blockSize = block.bufferOffset + meta_align_size(sizeof(Buffer) * count);
		block.pMappingAddr = (void**)BufferPool()->allocate(blockSize, 8);
		Buffer* pBuffer = (Buffer*)block.BufferPtr();
		void** ppMappingData = block.pMappingAddr;
		for (auto& field : fields) {
			new(pBuffer)Buffer{};
			BufferCreator creatorInfo{};
			creatorInfo.pBuffer = &pBuffer->buffer;
			creatorInfo.pAllocation = &pBuffer->allocation;
			creatorInfo.ppCpuData = ppMappingData;
			creatorInfo.size = field.type->size;
			creatorInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
			creatorInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			Backend::TransferWorker->CreateBuffer(creatorInfo);
			ppMappingData++;
			pBuffer++;
		}
	}
	void BindDescriptorSetBuffer(MaterialGpuResourceBlock& block, const ShaderDescriptorSet& descList, const VkDescriptorSetList& setList) {
		VkWriteDescriptorSet writeDescriptorSet[16] = {};
		VkDescriptorBufferInfo bufferInfoList[16] = {};
		uint32_t descCount = 0;
		Buffer* buffer = (Buffer*)block.BufferPtr();
		for (auto& desc : descList) {
			auto& bufferInfo = bufferInfoList[descCount];
			bufferInfo.offset = 0;
			bufferInfo.range = desc.size;
			bufferInfo.buffer = buffer->buffer;
			auto& writeDesc = writeDescriptorSet[descCount];
			writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDesc.dstSet = setList[desc.set];
			writeDesc.dstBinding = desc.binding;
			writeDesc.descriptorType = vkApiGetDescriptorType(desc.type);
			writeDesc.descriptorCount = 1;
			writeDesc.pBufferInfo = &bufferInfo;
			descCount++;
		}
		vkUpdateDescriptorSets(VulkanAPI::Ptr()->GetBackend().GetDevice().Ptr(), descCount, writeDescriptorSet, 0, nullptr);
	}
	
	MaterialInfo& FindMaterialStaticBlock(const refl::UClass* meta, std::span<const refl::FieldPtr> fields) {
		auto it = MaterialInfoTable.find(meta->name);
		if (it != MaterialInfoTable.end()) {
			return it->second;
		}

		uint16_t classMember = 0, staticMember = 0;
		for (auto& field : fields) {
			if (field.IsStatic()) {
				break;
			}
			classMember++;
		}
		MaterialResourceBlock staticBlock = MaterialResourceBlock::FromFields(fields.subspan(classMember, fields.size() - classMember));
		MaterialInfo info{ {},{}, staticBlock };
		MaterialInfoTable.emplace(meta->name, info);
		return MaterialInfoTable[meta->name];
	}
	void VulkanGlslLoader::LoadShaderBuffer(VkDescriptorSetList& setList, pmr::vector<api::ShaderProgram*>& programList, MaterialInfo& material)
	{
		if (material.isInit) {
			return;
		}
		material.isInit = true;
		auto meta = refl::find_info(programList[0]->ubName());
		if (!meta) {
			return;
		}
		auto fields = meta->GetFields(refl::FIND_ALL_MEMBER_WITH_STATIC, Name(""));
		auto& info = FindMaterialStaticBlock(meta, fields);
		MaterialResourceBlock classBlock = MaterialResourceBlock::FromFields(fields.subspan(0, fields.size() - info.staticBlock.count));
		if (!info.gpuBlock) {
			ShaderDescriptorSet descList{ FramePool() };
			GlslToSpirv::GetShaderLayout(descList, programList);
			InitMaterialGpuResourceBlock(info.gpuBlock, fields);
			BindDescriptorSetBuffer(info.gpuBlock, descList, setList);
		}
		material.classBlock = classBlock;
		material.gpuBlock = info.gpuBlock;
		material.staticBlock = info.staticBlock;
	}
	void vkShaderProgram::Load(const pmr::vector<uint32_t>& spirv)
	{
		VulkanAPI* API = VulkanAPI::Ptr();
		mPtr = API->GetBackend().GetDevice().CreateShaderModule(spirv);
		GlslToSpirv::LoadShaderLayout(this, spirv);
	}
	VkShaderStageFlagBits vkShaderProgram::GetVkStage()
	{
		switch (mStage) {
		case ShaderStage::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		default:
			return VkShaderStageFlagBits();
		}
	}
}

