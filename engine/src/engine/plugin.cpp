#ifndef ZLIB_API_VAL
#define ZLIB_API_VAL 1
#include "pmr/name.h"
#include "pmr/frame_allocator.h"
#endif // !ZLIB_API_VAL

#ifndef CORE_API_VAL
#define CORE_API_VAL 1
#include "zlog.h"
#include "module_manager_impl.inl"
#include "file_manager_impl.inl"
IMPLEMENT_STATIC_MODULE(CORE_API, api::CoreModule, core)
#endif // !CORE_API_VAL

#ifndef ASSET_API_VAL
#define ASSET_API_VAL 1
#include "resource_system_impl.inl"
#include "asset_visit_impl.inl"
IMPLEMENT_STATIC_MODULE(ASSET_API, api::AssetModule, asset)
#endif // !ASSET_API_VAL

#ifndef RENDER_API_VAL
#define RENDER_API_VAL 1
#include "renderapi_impl.inl"
#include "window_impl.inl"
#include "render/render_module.h"
IMPLEMENT_STATIC_MODULE(RENDER_API, api::RenderModule, render);
#endif // !RENDER_API_VAL

#ifndef APP_API_VAL
#define APP_API_VAL 1
#include "app_module.h"
#include "app_impl.inl"
#include "event_system_impl.inl"
#include "scene_system_impl.inl"
IMPLEMENT_STATIC_MODULE(APP_API, api::AppModule, app)
#endif // !APP_API_VAL

#ifndef UI_API_VAL
#define UI_API_VAL 1
#include "ui_render_system_impl.inl"
IMPLEMENT_STATIC_MODULE(UI_API, api::UIModule, ui)
#endif // !UI_API_VAL
