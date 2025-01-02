target("singleton")
    set_kind("shared")
    set_group("3rdparty")
    add_rules("engine.api")
    add_includedirs("include", {public = true})
    add_headerfiles("include/*.h")
    add_files("src/*.cpp")