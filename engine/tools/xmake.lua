function tool_target(name, kind)
    target(name)
        set_kind(kind or "binary")
        set_group("Tools")
        add_rules("engine.tool")
end
includes("*/xmake.lua")