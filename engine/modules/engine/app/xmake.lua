static_component("app","engine")
    add_headerfiles("include/**.h", "include/**.inl")
    add_files("src/**.cpp")
    add_deps("core", "asset", "zlib", "render")