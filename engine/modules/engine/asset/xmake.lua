static_component("asset","engine")
    add_rules("c++.codegen",{
        files = {"include/asset/res/*.h"}
    })
    add_headerfiles("include/**.h","include/**.inl")
    add_files("src/**.cpp")
    add_deps("core", "zlib")
    add_syslinks("Ole32",{public = true})