static_component("core","engine")
    add_rules("c++.codegen",{
        files = {"include/module/module.h"}
    })
    add_includedirs("3rdparty", {public = true})
    add_headerfiles("include/**.h","include/**.inl","3rdparty/**.h")
    add_files("src/**.cpp")
    add_deps("zlib")
    add_defines("YAML_CPP_STATIC_DEFINE", {public = true})
    add_packages("spdlog", {public = true})