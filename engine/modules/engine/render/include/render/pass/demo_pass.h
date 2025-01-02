#pragma once
#include "render_pass.h"
#include "render/graph/frame_graph.h"
namespace api {
	class DemoPass : public RenderPass {
	public:
		static void Setup(FrameGraph& graph, RenderPassBuilder& builder);
		static void Execute(FrameGraph&, RenderPassContext&);
	};

}