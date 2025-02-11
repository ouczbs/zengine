#pragma once
#include "object/game_object.h"
namespace api {
	class Scene;
	class LevelBlueprint {
	public:
		LevelBlueprint();
		~LevelBlueprint();
		void LoadScene(Scene* scene);
	};
}