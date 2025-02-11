#include "event/event_system.h"
#include "scene/scene_system.h"
#include "app_module.h"
namespace api {
	void AppModule::OnLoad(int argc, char** argv)
	{
		AddSystem<EventSystem>();
		AddSystem<SceneSystem>();
	}
	void AppModule::OnUnload()
	{

	}
}