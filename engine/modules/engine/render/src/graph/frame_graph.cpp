#include "render/graph/frame_graph.h"
#include "render/render_api.h"
#include <stack>
#include <unordered_set>
#include <algorithm>
namespace api {
	//先准备再执行，这样就可以在执行之前，做一些特殊的资源初始化工作
	FrameGraphNodePtr FrameGraph::AddRenderPass(const RenderPassSetupFunction& setup, const RenderPassNodeExecuteFn& executor)
	{
		FrameGraphNodePtr node_ptr{ setup, executor};
		mNodes.push_back(node_ptr);
		return node_ptr;
	}
	RenderPassBuilder FrameGraph::CreateRenderPassBuild()
	{
		return RenderPassBuilder{this};
	}
	void FrameGraph::InitSurface(TextureDesc* surfaces, uint32_t frames)
	{
		mTexturePool.reserve(frames + 1);
		mTexturePool.push_back(TextureDesc{});
		for (uint32_t i = 0; i < frames;i++) {
			surfaces[i].id = mTexturePool.size() + 1;
			mTexturePool.push_back(surfaces[i]);
		}
	}
	void FrameGraph::Setup()
	{
		uint32_t size = mNodes.size();
		for (uint32_t i = 0; i < size;i++) {
			auto& node = mNodes[i];//mNodes 数量可能会增加
			RenderPassBuilder builder{this, node};
			node->setup(*this, builder);
		}
	}
	void FrameGraph::Compile()
	{
		CullGraph();
		FillGraph();
	}
	void FrameGraph::CullGraph()
	{
		pmr::vector<RenderPassNode*> outputNodes{FramePool()};
		for (auto& node : mNodes) {
			if (node->IsOutput()) {
				outputNodes.push_back(node.node);
				if ((!mLastOutputNode || node->type > mLastOutputNode->type)) {
					mLastOutputNode = node.node;
				}
				if ((!mFirstInputNode || node->type < mFirstInputNode->type)) {
#ifdef WITH_EDITOR
					if(!mIsRenderEditorSurface || node->type >= RenderPassNodeType::Imgui)
#endif // WITH_EDITOR
					mFirstInputNode = node.node;
				}
			}
		}
		if (outputNodes.size() > 1) {
			std::sort(outputNodes.begin(), outputNodes.end(), [](const RenderPassNode* a, const RenderPassNode* b) {
				if (a->type != b->type) {
					return a->type < b->type;
				}
				return a < b;
				});
			for (size_t i = 1; i < outputNodes.size(); ++i) {
				outputNodes[i]->dependencies.push_back(outputNodes[i - 1]);
			}
		}
		//todo 需要考虑拓扑排序
		std::stack<RenderPassNode*> stack;
		stack.push(mLastOutputNode);
		while (!stack.empty()) {
			RenderPassNode* node = stack.top();
			stack.pop();
			if (node->isActive) {
				continue;
			}
			node->isActive = true;
			for (auto& rely : node->dependencies) {
				if (!rely->isActive) {
					stack.push(rely);
				}
			}
		}
		auto end = std::remove_if(mNodes.begin(), mNodes.end(), [this](FrameGraphNodePtr& node) {
			return !node->isActive;
		});
		mNodes.erase(end, mNodes.end());
	}
	void FrameGraph::FillGraph()
	{
		std::pmr::unordered_set<RenderPassNode*> seenNodes{FramePool()};
		pmr::vector<RenderPassNode*> result{ FramePool() };
		for (auto& node : mNodes) {
			bool isChange = false;
			for (auto& rely : node->dependencies) {
				bool isInsert = seenNodes.insert(rely).second;
				isChange = isChange || !isInsert;
				if (isInsert) {
					result.push_back(rely);
				}
			}
			if(isChange)
				node->dependencies = result;
			result.clear();
			seenNodes.clear();
		}
		std::sort(mNodes.begin(), mNodes.end(), [](const FrameGraphNodePtr& a, const FrameGraphNodePtr& b) {
			if (a->type != b->type) {
				return a->type < b->type;
			}
			return a < b;
		});
		if (mLastOutputNode) {
			mFirstInputNode->flag |= RenderPassNodeFlag::FirstInput;
			mLastOutputNode->flag |= RenderPassNodeFlag::LastOutput;
		}
	}
	TextureDesc& FrameGraph::GetRenderSurface()
	{
#ifdef WITH_EDITOR
		return ResolveTexture(mIsRenderEditorSurface ? mEditorSurfaceID: mSurfaceID);
#else
		return ResolveTexture(mSurfaceID);
#endif // WITH_EDITOR
	}
	void FrameGraph::Execute(FRenderView& view)
	{
		for (auto& node : mNodes) {
			mCurrentNode = node;
			switch (node.type) {
				case RenderPassType::Render:
					ExecuteRenderPass(node.node, view);
					break;
				case RenderPassType::Compute:
					ExecuteComputePass(node.node, view);
					break;
				case RenderPassType::Copy:
					ExecuteCopyPass(node.node, view);
					break;
				default:
					break;
			}
		}
		ExecutePresentPass(view);
	}
	void FrameGraph::Clear()
	{
		mNodes.clear();
		mFirstInputNode = nullptr;
		mLastOutputNode = nullptr;
		mTickStamp++;
	}
	void FrameGraph::ExecuteRenderPass(RenderPassNode* node, FRenderView& view)
	{
		RenderAPI::Ptr()->BeginRenderPass(node, [](RenderPassNode* node) {
			ExecuteResourceBarriers(node, RenderPassType::Render);
		});
		RenderPassContext context{view.context, node};
		std::get<RenderPassExecuteFunction>(node->executor)(*this, context);
		RenderAPI::Ptr()->EndRenderPass(node);
	}
	void FrameGraph::ExecutePresentPass(FRenderView& view)
	{
		TextureDesc& surface = GetRenderSurface();
		if (surface.state == ResourceState::PRESENT) {
			return;
		}
		ResourceState srcState = surface.state;
		surface.state = ResourceState::PRESENT;
		TextureBarrier barrier{};
		barrier.mSrcState = srcState;
		barrier.mDstState = ResourceState::PRESENT;
		barrier.mTexture = surface;
		ResourceBarrierDesc desc{};
		desc.textureBarriersCount = 1;
		desc.pTextureBarriers = &barrier;
		view.context->ExecuteSurfaceBarriers(desc);
	}
	void FrameGraph::ExecuteComputePass(RenderPassNode* node, FRenderView& view)
	{
		ComputePassContext context{};
		std::get<ComputePassExecuteFunction>(node->executor)(*this, context);
	}
	void FrameGraph::ExecuteCopyPass(RenderPassNode* node, FRenderView& view)
	{
		CopyPassContext context{};
		std::get<CopyPassExecuteFunction>(node->executor)(*this, context);
	}
	void FrameGraph::ExecuteResourceBarriers(RenderPassNode* node, RenderPassType type)
	{
		pmr::vector<BufferBarrier>  bufferBarrier{FramePool()};
		pmr::vector<TextureBarrier> textureBarrier{ FramePool() };
		auto graph = &RenderAPI::Ptr()->graph;
		node->ForeachEdge([&](FrameResource* resource) {
			std::visit([&](auto& desc) {
				ResourceState srcstate, dststate;
				if (!graph->ResolveState(desc, srcstate, dststate)) {
					auto barrier = desc.ToBarrier(srcstate, dststate);
					using T = decltype(barrier);
					if constexpr (std::is_same_v<T, BufferBarrier>) {
						bufferBarrier.push_back(barrier);
					}
					else {
						textureBarrier.push_back(barrier);
					}
				}
				}, resource->res);
			});
		if (bufferBarrier.empty() && textureBarrier.empty()) {
			return;
		}
		ResourceBarrierDesc desc{};
		desc.bufferBarriersCount = bufferBarrier.size();
		desc.pBufferBarriers = bufferBarrier.data();
		desc.textureBarriersCount = textureBarrier.size();
		desc.pTextureBarriers = textureBarrier.data();
		RenderAPI::Ptr()->context.ExecuteResourceBarriers(desc);
	}
	void FrameGraph::TransitionState(TextureDesc& desc, ResourceState state)
	{
		TextureDesc& texture = ResolveTexture(desc.id);
		desc.state = state;
		texture.state = state;
	}
	bool FrameGraph::ResolveState(TextureDesc& desc, ResourceState& srcstart, ResourceState& dststate)
	{
		TextureDesc& texture = ResolveTexture(desc.id);
		srcstart = texture.state;
		dststate = desc.state;
		texture.state = dststate;
		desc.image = texture.image;
		return srcstart == dststate;
	}
	bool FrameGraph::ResolveState(BufferDesc& desc, ResourceState& srcstart, ResourceState& dststate)
	{
		return true;
	}
	TextureDesc& FrameGraph::ResolveTexture(int32_t id)
	{
		if (id <= 0 || id > mTexturePool.size()) {
			return mTexturePool[0];//empty
		}
		TextureDesc& texture = mTexturePool[id - 1];
		if (!texture.image) {
			RenderAPI::Ptr()->CreateTexture(texture);
		}
		return texture;
	}
	ImageViewPtr FrameGraph::ResolveTextureView(TextureDesc& desc)
	{
		if (!desc.image) {
			TextureDesc& texture = ResolveTexture(desc.id);
			desc.image = texture.image;
		}
		return ResolveTextureView(desc.ToTextureView());
	}
	ImageViewPtr FrameGraph::ResolveTextureView(TextureViewKey key)
	{
		auto it = mTextureViewPool.find(key);
		if (it != mTextureViewPool.end()) {
			return it->second;
		}
		ImageViewPtr view = RenderAPI::Ptr()->CreateTextureView(key);
		mTextureViewPool.emplace(key, view);
		return view;
	}
	void* FrameGraph::ResolveTextureSampler(TextureSampler sampler)
	{
		auto iter = mTextureSamplerPool.find(sampler);
		if (iter != mTextureSamplerPool.end()) {
			return iter->second;
		}
		return RenderAPI::Ptr()->CreateTextureSampler(sampler);
	}
	TextureDesc FrameGraph::ResourceTexture(Name name, int num)
	{
		Tag tag(name, num);
		auto it = mTextureTagMap.find(tag);
		if (it != mTextureTagMap.end()) {
			return ResolveTexture(it->second);
		}
		return mTexturePool[0];
	}
	uint32_t FrameGraph::GetTextureID(Name name, int num)
	{
		Tag tag(name, num);
		auto it = mTextureTagMap.find(tag);
		if (it != mTextureTagMap.end()) {
			return it->second;
		}
		return 0;
	}
	void FrameGraph::AcquireTexture(TextureDesc& desc)
	{
		auto it = mTextureKeyMap.find(desc);
		if (it != mTextureKeyMap.end()) {
			for (auto& id : it->second) {
				if (id.tick != mTickStamp) {
					id.tick = mTickStamp;
					desc.id = id.id;
					return;
				}
			}
		}
		desc.id = mTexturePool.size() + 1;
		TextureID id{ desc.id , mTickStamp };
		mTexturePool.push_back(desc);
		mTextureKeyMap[desc].push_back(id);
	}
	void FrameGraph::RealeaseTexture(TextureDesc& desc)
	{
		auto it = mTextureKeyMap.find(desc);
		if (it != mTextureKeyMap.end()) {
			for (auto& id : it->second) {
				if (id.id == desc.id) {
					id.tick = mTickStamp - 1;
					desc.id = 0;
					return;
				}
			}
		}
	}
	void FrameGraph::SetResourceTexture(TextureDesc desc, Name name, int num)
	{
		Tag tag(name, num);
		auto it = mTextureTagMap.find(tag);
		if (it != mTextureTagMap.end()) {
			//todo: destroy texture
			desc.id = it->second;
			mTexturePool[desc.id - 1] = desc;
		}
		else {
			desc.id = mTexturePool.size() + 1;
			mTexturePool.push_back(desc);
			mTextureTagMap[tag] = desc.id;
		}
	}
}
