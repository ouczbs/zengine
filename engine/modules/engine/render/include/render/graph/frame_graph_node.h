#pragma once
#include "meta/enum.h"
#include "render/render_type.h"
namespace api{
    class FrameGraph;
	class RenderPassBuilder;
	struct RenderPassNode;
	struct RenderContext;
	struct ComputePassContext {};
	struct RenderPassContext {
		RenderContext* parent;
		RenderPassNode* node;
		RenderPassContext(RenderContext* parent, RenderPassNode* node) : parent(parent),node(node) {};
		RenderContext* operator->() {
			return parent;
		}
		size_t PassKey()const;
	};
	struct CopyPassContext {};
	using RenderPassSetupFunction = std::function<void(FrameGraph&, RenderPassBuilder&)>;
	using CopyPassExecuteFunction = std::function<void(FrameGraph&, CopyPassContext&)>;
	using ComputePassExecuteFunction = std::function<void(FrameGraph&, ComputePassContext&)>;
	using RenderPassExecuteFunction = std::function<void(FrameGraph&, RenderPassContext&)>;
	using RenderPassNodeExecuteFn = std::variant<RenderPassExecuteFunction, ComputePassExecuteFunction, CopyPassExecuteFunction>;
	struct FrameGraphNodePtr {
		using NodeType = RenderPassType;
		RenderPassNode* node;
		NodeType	type{ NodeType::Render };
		FrameGraphNodePtr() : node(nullptr){};
		FrameGraphNodePtr(const RenderPassSetupFunction& setup, const RenderPassNodeExecuteFn& executor, NodeType type = NodeType::Render);
		operator bool() const {
			return node;
		}
		RenderPassNode* operator ->()const {
			return node;
		}
	};
	struct FrameResource {
		using Resource = std::variant<TextureDesc, BufferDesc>;
		Name								name;
		Resource							res;
		FrameResource() noexcept = default;
		FrameResource(Name name,const Resource& res) : name(name), res(res){};
		template<typename T>
		T& CastTo() {
			return std::get<T>(res);
		}
		bool IsTexture() const{
			return std::holds_alternative<TextureDesc>(res);
		}
		bool IsBuffer() const {
			return std::holds_alternative<BufferDesc>(res);
		}
		static FrameResource* Make(Name name, const Resource& res) {
			return new(FramePool()) FrameResource(name, res);
		}
	};
	using FrameGraphEdgePtr = FrameResource*;
	using RenderPassEdgeIterFn = std::function<void(FrameResource*)>;
	struct RenderPassNode {
		Name	name;
		size_t  hash{0};
		RenderPassNodeType type{0};
		RenderPassNodeFlag flag{0};
		bool     isActive{ false };
		RenderPassParams  params{};
		RenderPassSetupFunction setup;
		RenderPassNodeExecuteFn executor;
		pmr::vector<RenderPassNode*>   dependencies{ FramePool() };
		pmr::vector<FrameGraphEdgePtr> inEdges{ FramePool() };
		pmr::vector<FrameGraphEdgePtr> outEdges{ FramePool() };
		void ForeachEdge(RenderPassEdgeIterFn fn);
		bool IsOutput() { return any(flag & RenderPassNodeFlag::Output); }
		bool IsFirstInput() { return any(flag & RenderPassNodeFlag::FirstInput); }
		bool IsLastOutput() { return any(flag & RenderPassNodeFlag::LastOutput); }
		bool IsWaitTransfer() { return any(flag & RenderPassNodeFlag::WaitTransfer); }
		void SetWaitTransfer() { flag |= RenderPassNodeFlag::WaitTransfer; }
		FrameGraphEdgePtr GetInput(int i) { return inEdges[i]; };
		FrameGraphEdgePtr GetOutput(int i) { return outEdges[i]; };
		FrameGraphEdgePtr FindInput(Name name)
		{
			for (auto& edge : inEdges) {
				if (edge->name == name) {
					return edge;
				}
			}
			return {};
		}
		FrameGraphEdgePtr FindOutput(Name name)
		{
			for (auto& edge : outEdges) {
				if (edge->name == name) {
					return edge;
				}
			}
			return {};
		}
	};
	inline size_t RenderPassContext::PassKey()const
	{
		return node->hash;
	}
	struct RenderPassBuilder {
		FrameGraph& graph;
		FrameGraphNodePtr node;
		FrameResource* resource{ nullptr };
	public:
		RenderPassBuilder(FrameGraph* graph) noexcept
			: graph(*graph), node() {};
		RenderPassBuilder(FrameGraph* graph, FrameGraphNodePtr& node) noexcept
			: graph(*graph), node(node) {};
		RenderPassBuilder& Name(pmr::Name name);
		RenderPassBuilder& Type(RenderPassNodeType type, RenderPassNodeFlag flag = RenderPassNodeFlag::None);
		RenderPassBuilder& Read(TextureDesc desc, ResourceState state, pmr::Name name = {});
		RenderPassBuilder& Write(TextureDesc desc, ResourceState state, pmr::Name name = {});
		RenderPassBuilder& Dependency(RenderPassNode* rely) { node->dependencies.push_back(rely); return *this; }
		RenderPassBuilder& Attachment(AttachmentFlag flag);
	};
}