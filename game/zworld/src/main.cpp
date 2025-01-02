#include "xmalloc_new_delete.h"
#include "SDL.h"
#include "engine/api.h"
#include "zlog.h"
#include <iostream>
int main(int argc,char* argv[]) {
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_EVENTS) < 0) {
        zlog::errorf("SDL_Init:: {}", SDL_GetError());
    }
    auto ptr = api::ModuleManager::Ptr();
    ptr->MakeGraph("zworld", true, argc, argv);
    ptr->MainLoop();
    ptr->DestroyGraph();
    SDL_Quit();
    return 0;
}