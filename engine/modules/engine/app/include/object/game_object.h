#pragma once
#include "archive/reflect.h"
namespace api {
	using pmr::Name;
	using refl::TAny;
	using std::vector;
	class Component;
	class GameObject {
	private:
		GENERATED_BODY()
		UPROPERTY()
		Name mName;
		UPROPERTY()
		vector<TAny<GameObject>> mChildrens;
		UPROPERTY()
		vector<TAny<Component>>  mComponents;
	public:

	};
	class Component {
	private:
		GENERATED_BODY()
		UPROPERTY()
		Name mName;
		GameObject*		mOwner;

	public:

	};
}
#include ".app/game_object_gen.inl"