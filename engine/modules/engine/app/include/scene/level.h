#pragma once
#include "object/game_object.h"
#include "math/vector3.h"
namespace api {
	using refl::TAny;
	struct LevelInfo {
		UPROPERTY()
		Name	mName;
		UPROPERTY()
		Name	mPath;
		UPROPERTY()
		Vector3 mOffset;
	};
	class Level : public LevelInfo{
	protected:
		GENERATED_BODY()
		UPROPERTY()
		vector<TAny<GameObject>>  mObjects;
	public:
		Level(const LevelInfo& info) : LevelInfo(info) {};
		Level();
		~Level();
		void AddObject(TAny<GameObject> object);
	};
}
#include ".app/level_gen.inl"