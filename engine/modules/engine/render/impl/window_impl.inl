#include "render/window.h"
namespace api {
    SINGLETON_DEFINE(Window)
	inline Window::Window(const Args& args, int width, int height) noexcept : mHeight(height), mWidth(width)
	{
        SINGLETON_PTR();
        uint32_t windowFlags = args.windowFlags | SDL_WINDOW_SHOWN;
        windowFlags |= args.resizeable ? SDL_WINDOW_RESIZABLE : 0;
        windowFlags |= args.headless ? SDL_WINDOW_HIDDEN : 0;
        // Even if we're in headless mode, we still need to create a window, otherwise SDL will not poll events.
        mPtr = SDL_CreateWindow(args.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);
	}
}