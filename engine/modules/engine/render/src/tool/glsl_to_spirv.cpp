#include "render/tool/glsl_to_spirv.h"
#include "render/asset/ubo.h"
#include "render/render_api.h"
#include "zlog.h"
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>
namespace api
{
	static pmr::table<Guid, ShaderDescriptorSet> ShaderLayoutTable;
	optional<pmr::vector<uint32_t>> GlslToSpirv::ToSpirv(const pmr::string& glsl, shaderc_shader_kind kind, string_view code_id)
	{
		optional<pmr::vector<uint32_t>> spirv_out{FramePool()};
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
			options.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
			auto result = compiler.CompileGlslToSpv(glsl.data(), kind, code_id.data(), options);
			if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
				auto err_m = result.GetErrorMessage();
				auto err_msg = err_m.c_str();
				zlog::error("load spirv failed!!! {}", err_msg);
				return spirv_out;
			}
			spirv_out = pmr::vector<uint32_t>{ result.cbegin(),result.cend() };
		}
		return spirv_out;
	}
	shaderc_shader_kind GlslToSpirv::GetShaderKind(Name name)
	{
		static Name vert(".vert");
		static Name frag(".frag");
		if (name == vert) {
			return shaderc_shader_kind::shaderc_vertex_shader;
		}
		if(name == frag) {
			return shaderc_shader_kind::shaderc_fragment_shader;
		}
		throw std::runtime_error("unsupport type");
	}
	ShaderStage GlslToSpirv::GetShaderStage(shaderc_shader_kind kind)
	{
		switch (kind)
		{
		case shaderc_shader_kind::shaderc_vertex_shader:
			return ShaderStage::VERTEX;
		case shaderc_shader_kind::shaderc_fragment_shader:
			return ShaderStage::FRAGMENT;
		default:
			return ShaderStage::NONE;
		}
	}
	void GlslToSpirv::LoadShaderLayout(ShaderProgram* program, const pmr::vector<uint32_t>& spirv) {
		if (ShaderLayoutTable.find(program->GetGuid()) != ShaderLayoutTable.end()) {
			return;
		}
		auto meta = refl::find_info(program->ubName());
		using FieldPtrSpan = std::span<const refl::FieldPtr>;
		FieldPtrSpan fieldList = meta ? meta->GetFields(refl::FIND_ALL_MEMBER, Name("")) : FieldPtrSpan{};
		struct FindFieldResult {
			const refl::FieldPtr* field;
			ubMetaInfo info;
			operator bool() {
				return field;
			}
		};
		auto FindFieldFn = [](FieldPtrSpan fieldList, uint32_t set, uint32_t binding) -> FindFieldResult{
			for (auto& field : fieldList) {
				ubMetaInfo info = ubMetaInfo::FromField(field);
				if (info.set == set && info.binding == binding) {
					return { &field , info};
				}
			}
			return {};
		};
		ShaderStage stage = program->GetStage();
		spirv_cross::Compiler compiler(spirv.data(), spirv.size());
		auto resources = compiler.get_shader_resources();
		auto InitDescriptorLauyoutFn = [=,&FindFieldFn, &compiler](const spirv_cross::Resource& res)->ShaderDescriptorLayout {
			ShaderDescriptorLayout layout{};
			layout.set = compiler.get_decoration(res.id, spv::Decoration::DecorationDescriptorSet);
			layout.binding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
			layout.stageFlags = stage;
			if (auto result = FindFieldFn(fieldList, layout.set, layout.binding)) {
				layout.name = result.field->name;
			}
			return layout;
		};
		ShaderDescriptorSet descriptorSet;
		// 遍历 uniform buffers (UBO)
		for (auto& res : resources.uniform_buffers)
		{
			ShaderDescriptorLayout layout = InitDescriptorLauyoutFn(res);
			auto type = compiler.get_type(res.type_id);
			uint32_t size = type.width;
			uint32_t i = 0;
			for (auto& member_type : type.member_types)
			{
				auto tmp = compiler.get_type(member_type);
				size += (uint32_t)(compiler.get_declared_struct_member_size(type, i++));
			}
			layout.size = size;
			layout.type = ShaderDescriptorType::UNIFORM_BUFFER;
			descriptorSet.push_back(layout);
		}
		// 遍历 sampled images (采样器/纹理)
		for (auto& res : resources.sampled_images) {
			ShaderDescriptorLayout layout = InitDescriptorLauyoutFn(res);
			layout.type = ShaderDescriptorType::SAMPLER;
			descriptorSet.push_back(layout);
		}
		ShaderLayoutTable.emplace(program->GetGuid(), descriptorSet);
	}
	void GlslToSpirv::GetShaderLayout(ShaderDescriptorSet& descriptorSet, pmr::vector<ShaderProgram*>& programList)
	{
		table<uint32_t, uint32_t> idTable{ FramePool() };
		for (auto program : programList) {
			auto& itSet = ShaderLayoutTable[program->GetGuid()];
			for (ShaderDescriptorLayout& layout : itSet) {
				uint32_t id = (layout.set << 8) + layout.binding;
				auto itId = idTable.find(id);
				if (itId == idTable.end()) {
					descriptorSet.emplace_back(layout);
					idTable[id] = descriptorSet.size() - 1;
				}
				else {
					using ::operator|=;
					ShaderDescriptorLayout& desc = descriptorSet[itId->second];
					desc.stageFlags |= layout.stageFlags;
				}
			}
		}
		std::sort(descriptorSet.begin(), descriptorSet.end(), [](auto& k1, auto& k2) {
				if (k1.set != k2.set) {
					return k1.set < k2.set;
				}
				return k1.binding < k2.binding;
			});
	}
}
