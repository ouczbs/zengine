#include "refl/pch.h"
#include <iostream>
namespace api {
	struct Guid {
		UPROPERTY()
		int a;
		UPROPERTY()
		float b;
		UPROPERTY()
		std::string view;
		Guid() {}
		USING_OVERLOAD_CTOR(Guid, int , float)
		UFUNCTION({}, ref = USING_CTOR_NAME)
		Guid(int aa, float bb) :a(aa), b(bb), view("default") {
			std::cout << view << std::endl;
		}
		UFUNCTION({})
		int Multy(int c)const {
			int d = a * b;
			return d * c;
		}
	};
}
#include ".zworld-editor/test_refl_gen.inl"