header_component("zlib","engine")
    set_basename("myzlib") 
    add_headerfiles("include/**.h", "include/**.inl")
    if is_mode("debug") then 
        add_defines("API_DEBUG", {public = true})
    end
    add_defines("NOMINMAX", {public = true})
    add_syslinks("kernel32")
    set_pcheader("include/refl/pch.h")
    add_deps("xmalloc", {public = true})