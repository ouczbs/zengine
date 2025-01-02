#pragma once
#include <SDL.h>
#include "pmr/frame_allocator.h"
namespace api {
	class RENDER_API Window {
	protected:
		using CreatePFN = decltype(&SDL_CreateWindow);
		int mWidth;
		int mHeight;
		SDL_Window* mPtr = nullptr;
		struct Args {
			const char* title;
			uint32_t windowFlags = 0;
			bool resizeable = true;
			bool headless = false;
		};
		SINGLETON_IMPL(Window)
	public:
		void* operator new(size_t size) {
			return ::operator new(size, GlobalPool());
		}
		void operator delete(void* p) {}
	public:
		Window(const Args& args, int width, int height) noexcept;
		SDL_Window* GetPtr() { return mPtr; }
	};
};