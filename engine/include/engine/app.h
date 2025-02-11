#pragma once 
#include "data/global.h"
namespace api{
    class ENGINE_API App {
	public:
		void* operator new(size_t size) {
			return ::operator new(size, GlobalPool());
		}
		virtual bool Launch();
		virtual void Update();
	};
}