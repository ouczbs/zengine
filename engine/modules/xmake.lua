function header_component(name, owner, opt)
    target(owner)
        add_deps(name)
    target_end()
    target(name)
        set_kind("headeronly")
        set_group("Engine/"..owner.."__comp")
        add_rules("engine.api")
        add_includedirs("include", {public = true})
        add_deps("singleton")
end
function static_component(name, owner, opt)
    target(owner)
        add_deps(name)
        add_includedirs("impl")
        add_headerfiles("impl/*.inl")
    target_end()
    target(name)
        set_kind("static")
        set_group("Engine/"..owner.."__comp")
        add_rules("engine.api")
        add_includedirs("include", {public = true})
        add_deps("singleton")
end
function shared_module(name, owner, opt)
    target(name)
        add_defines(string.upper(name).."_ROOT="..os.curdir():gsub("\\", "\\\\"),{public = false})
        set_kind("shared")
        set_group("Engine/"..owner.."__dyn")
        add_rules("engine.api")
        add_includedirs("include", {public = true})
        --add_rules("engine.plugin", {file = opt and opt.file or "include/" .. name .. "/module.h"})
end
function add_dependency(...)
    add_deps(...)
    local args = {...}
    local opt = args[#args]
    if type(opt) ~= "table" or not opt.public then 
        return
    end
    for _,v in ipairs(args) do 
        if v ~= opt then
            add_values("module.public_dependencies", v)
        end
    end
end
function game_instance(name, opt)
    target(name)
        set_rundir(".")
        set_kind("binary")
        set_group("Games")
        add_rules("engine.api")
        --add_rules("engine.plugin", {file = opt and opt.file or "src/" .. name .. ".h"})
        add_defines("SDL_MAIN_HANDLED")
    target(name .. "-editor")
        set_kind("binary")
        set_group("Games")
        --add_deps(name)
end
includes("**/xmake.lua")