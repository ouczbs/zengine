-- tool_target("make_plugin", "shared")
--     add_includedirs("src")
--     add_files("src/*.cpp")
--     add_headerfiles("src/*.h")
--     add_deps("core")
--     set_runargs(os.projectdir(), [[build\.gens\vulkan\windows\xmake.lua]], 
--     os.projectdir() .. [[\engine\modules\render\vulkan]], [[include\vulkan\module.h]], "vulkan" .. ".plugin.cpp")