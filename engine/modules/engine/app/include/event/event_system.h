#pragma once
#include "event.h"
#include "module/module.h"
namespace api {
	class FrameGraph;
	class APP_API EventSystem : public ISystem{
		SINGLETON_IMPL(EventSystem)
	public:
		EventSystem();
		void Initialize() override;
		void Finalize() override;
		Event<void(FrameGraph&, uint32_t)> BeginRenderFrame{"BeginRenderFrame"};
	};
}