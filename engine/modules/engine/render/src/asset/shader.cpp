#include "render/asset/shader.h"
#include "render/tool/glsl_to_spirv.h"
namespace api {
	Shader::Shader() : Asset(meta_info<Shader>())
	{
	}
	Shader::~Shader()
	{
	}
	void Shader::BeginLoad()
	{
	}
}