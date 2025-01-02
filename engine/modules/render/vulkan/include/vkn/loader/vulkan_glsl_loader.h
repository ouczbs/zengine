#pragma once
#include "render/asset/shader.h"
#include "asset/resource_system.h"
#include "vkn/vulkan_api.h"
namespace vkn {
	class vkShaderProgram : public api::ShaderProgram {
	private:
		VkShaderModule mPtr;
	public:
		using api::ShaderProgram::ShaderProgram;
		VkShaderModule Ptr() {
			return mPtr;
		}
		void Load(const pmr::vector<uint32_t>& spirv);
		VkShaderStageFlagBits GetVkStage();
	};
	class VulkanGlslLoader : public api::IFileLoader
	{
		inline static table<Guid, void*> ShaderTable;
	public:
		static void Init();
		api::ResourceBundle LoadFile(api::PackagePath handle, const api::MetaBundle& meta) override;
		static uint32_t GetShaderLayout(VkDescriptorSetLayoutList& layoutList, pmr::vector<api::ShaderProgram*>& programList);
		static void LoadShaderBuffer(VkDescriptorSetList& setList, pmr::vector<api::ShaderProgram*>& programList, MaterialInfo& info);
	};
}