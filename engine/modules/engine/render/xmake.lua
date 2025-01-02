static_component("render","engine")
    add_rules("c++.codegen",{
        files = {"include/render/asset/*.h"}
    })
    add_includedirs("3rdparty", {public = true})
    add_headerfiles("include/**.h")
    add_files("src/**.cpp")
    add_deps("asset", "zlib", "core")
    add_syslinks("user32", {public = true})
    add_packages("libsdl","shaderc","spirv-cross", {public = true})
    if WITH_EDITOR then 
        add_defines("WITH_EDITOR", {public = true})
    end