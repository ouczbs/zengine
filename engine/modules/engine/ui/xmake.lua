static_component("ui","engine")
    add_headerfiles("include/**.h")
    add_files("src/**.cpp")
    add_deps("core", "asset", "zlib", "render")
    add_packages("noesis", "stb", {public = true})
    if WITH_EDITOR then 
        add_files("include/imgui/*.cpp")
        add_packages("imgui", {public = true})
    end