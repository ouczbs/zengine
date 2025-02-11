#include "data/global.h"
namespace api {
	APP_API EngineConfig  gEngineConfig{};
	APP_API ProjectConfig gProjectConfig{};
#ifdef WITH_EDITOR
	APP_API EditorConfig  gEditorConfig{};
#endif
}