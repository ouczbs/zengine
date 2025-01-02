#include "data/engine_config.h"
#include "event/event_system.h"
#include "app.h"
namespace api {
	void AppModule::OnLoad(int argc, char** argv)
	{
		AddSystem<EventSystem>();
	}
	void AppModule::OnUnload()
	{

	}
}