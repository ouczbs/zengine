#include "scene/level.h"

namespace api {
	Level::Level()
	{

	}
	Level::~Level()
	{

	}
	void Level::AddObject(TAny<GameObject> object)
	{
		mObjects.push_back(object);
	}
}
