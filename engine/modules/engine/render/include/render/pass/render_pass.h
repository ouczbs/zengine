#pragma once
#include "asset/res/resource_handle.h"
namespace api {
	class RenderPass {
		friend class FrameGraph;
	protected:
		Name name;
		pmr::vector<RscHandle<Texture>> mInputs;
		pmr::vector<RscHandle<Texture>> mOutputs;
	};
}