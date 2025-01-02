#include "render/render_api.h"
#include "render/render_module.h"
namespace api {
	SINGLETON_DEFINE(RenderAPI)
	void RenderAPI::RenderView(FRenderView& view)
	{
		view.context = &context;
		graph.Compile();
		graph.Execute(view);
		graph.Clear();
	}
	void RenderAPI::Render()
	{
		graph.Setup();
		for (auto view : context.views) {
			RenderView(view);
		}
	}
	RenderAPI::RenderAPI(RenderContext* ctx) : context(*ctx)
	{
		SINGLETON_PTR();
	}
	RenderAPI::~RenderAPI()
	{

	}
}