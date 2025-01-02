#include "render/graph/frame_graph.h"
#include "render/pass/render_pass.h"
namespace api {
	void RenderPassNode::ForeachEdge(RenderPassEdgeIterFn fn) {
		for (auto& edge : inEdges) {
			fn(edge);
		}
		for (auto& edge : outEdges) {
			fn(edge);
		}
	}
	FrameGraphNodePtr::FrameGraphNodePtr(const RenderPassSetupFunction& setup, const RenderPassNodeExecuteFn& executor, NodeType type) : type(type)
	{
		node = new (FramePool()) RenderPassNode();
		node->setup = setup;
		node->executor = executor;
	}
	RenderPassBuilder& RenderPassBuilder::Name(pmr::Name name)
	{
		node->name = name;
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::Type(RenderPassNodeType type, RenderPassNodeFlag flag)
	{
		node->type = type;
		node->flag = flag;
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::Read(TextureDesc desc, ResourceState state, pmr::Name name)
	{
		desc.state = state;
		FrameGraphEdgePtr edge = FrameResource::Make(name, desc);
		node->inEdges.push_back(edge);
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::Write(TextureDesc desc, ResourceState state, pmr::Name name)
	{
		desc.state = state;
		FrameGraphEdgePtr edge = FrameResource::Make(name, desc);
		node->outEdges.push_back(edge);
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::Attachment(AttachmentFlag flag)
	{
		uint32_t mask = node->outEdges.size();
		if (mask && any(flag)) {
			mask = 1 << (mask - 1);
			if (any(flag & AttachmentFlag::Clear)) {
				node->params.clear |= mask;
			}
			if (any(flag & AttachmentFlag::DiscardStart)) {
				node->params.discardStart |= mask;
			}
			if (any(flag & AttachmentFlag::DiscardEnd)) {
				node->params.discardEnd |= mask;
			}
			if (any(flag & AttachmentFlag::Sample)) {
				node->params.sampleMask |= mask;
			}
			if (any(flag & AttachmentFlag::MainPass)) {
				node->params.passMask |= mask;
			}
			if (any(flag & AttachmentFlag::Subpass)) {
				node->params.subpassMask |= mask;
			}
		}
		return *this;
	}

}