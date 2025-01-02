game_instance("zworld", "src/zworld.h")
target("zworld")
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
    add_dependency("engine", "editor", "vulkan", {public = true})
-- target("zworld-editor")
--     add_rules("c++.codegen",{
--         files = {"editor/test_refl.h"}
--     })
--     add_files("editor/*.cpp")
--     add_headerfiles("editor/*.h")
--     add_deps("zlib", "core", "xmalloc")