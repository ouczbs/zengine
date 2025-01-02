#pragma once
#include "render/type.h"
namespace api{
    struct EngineConfig {
		GraphicsAPI API = GraphicsAPI::Vulkan;
#ifdef WITH_EDITOR
		bool     IsRenderEditorSurface = false;
#endif // WITH_EDITOR

	};
}