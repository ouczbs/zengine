add_requires("spdlog", "vulkansdk","shaderc","spirv","spirv-cross","stb")
add_requires("mimalloc", {configs = {shared = true, debug = true, copy = true}})
add_requires("imgui",{configs = {shared = true, debug = true, copy = true}})
add_requires("noesis",{configs = {shared = true, copy = true}})
add_requires("libsdl",{configs = {shared = true}})
includes("*/xmake.lua")