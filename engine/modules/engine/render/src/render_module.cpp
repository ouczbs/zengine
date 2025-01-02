#include "render/render_module.h"
#include "render/asset/ubo.h"
#include "zlog.h"
namespace api {
	void RenderModule::OnLoad(int argc, char** argv)
	{
		refl::register_meta<ubSimple>();
	}
	void RenderModule::OnUnload()
	{

	}
}