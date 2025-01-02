shared_module("vulkan","engine")
    add_headerfiles("include/**.h","src/noesis/*.h")
    add_packages("vulkansdk", {public = true})
    add_dependency("engine", {public = true})
    if WITH_EDITOR then 
        add_deps("editor")
        add_headerfiles("src/imgui/*.h")
        add_files("src/**.cpp", "include/volk/volk.c")
    else 
        add_files("src/**.cpp|imgui/*|vulkan_imgui_editor.cpp", "include/volk/volk.c")
    end
    on_load(function (target)
        if is_plat("windows") then 
            target:add("defines", "VK_USE_PLATFORM_WIN32_KHR=1")
        elseif is_plat("linux") then 
            target:add("defines", "VK_USE_PLATFORM_XLIB_KHR=1")
        elseif is_plat("macosx") then 
            target:add("defines", "VK_USE_PLATFORM_MACOS_MVK=1")
        end
    end)