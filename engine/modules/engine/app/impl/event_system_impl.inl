#include "event/event_system.h"
namespace api {
	SINGLETON_DEFINE(EventSystem)

	inline EventSystem::EventSystem()
	{
		SINGLETON_PTR();
	}
	inline void EventSystem::Initialize()
	{

	}
	inline void EventSystem::Finalize()
	{

	}
}