static_component("app","engine")
    add_rules("c++.codegen",{
        files = {"include/object/**.h", "include/scene/*.h"}
    })
    add_headerfiles("include/**.h")
    add_files("src/**.cpp")
    add_deps("core", "asset", "zlib", "render")