#pragma once
#include "pmr/name.h"
#include "render/asset/shader.h"
#include <optional>
#include <shaderc/shaderc.h>
namespace api
{
	using std::optional;
	using std::string_view;
	class GlslToSpirv
	{
	public:
		static shaderc_shader_kind GetShaderKind(Name name);
		static ShaderStage GetShaderStage(shaderc_shader_kind kind);
		static optional<pmr::vector<uint32_t>> ToSpirv(const pmr::string& glsl, shaderc_shader_kind kind, string_view code_id = "unknown_shader");
		static void LoadShaderLayout(ShaderProgram* program, const pmr::vector<uint32_t>& spirv);
		static void GetShaderLayout(ShaderDescriptorSet& descriptorSet, pmr::vector<ShaderProgram*>& programList);
	};
}