import("core.project.project")
function add_define(target, name, is_static)
    local api = string.upper(name) .. "_API"
    if is_static then 
        target:add("defines", api .. "=", {public = false})
    else 
        target:add("defines", api.."=__declspec(dllimport)", {interface=true})
        target:add("defines", api.."=__declspec(dllexport)", {public=false})
    end
end
function get_private(target, name)
    local values = target:get(name)
    local extraconf = target:extraconf(name)
    if not extraconf then 
        return values
    end
    local results = {}
    for _,path in ipairs(values) do 
        local v = extraconf[path]
        if not v or (not v.public and not v.interface) then 
            table.insert(results, path)
        end
    end
    return results
end
function add_includedirs(target, deptarget, is_static)
    if not is_static then 
        local includedirs = get_private(deptarget, "includedirs")
        target:add("includedirs", includedirs)
    end
end
function is_static_f(kind)
    return kind == "static" or kind == "headeronly" or kind == "moduleonly"
end
function main(target)
    local name = target:name()
    local is_static = is_static_f(target:kind())
    add_define(target, name, is_static)
    local deps = target:get("deps")
    if not deps then return end
    for _,dep in ipairs(deps) do 
        local deptarget = project.target(dep)
        if is_static_f(deptarget:kind()) then
            add_define(target, dep, is_static)
            add_includedirs(target, deptarget, is_static)
        end
    end
end