function find_exe_dir()
    local os_name = os.host()
    local arch_name = os.arch()
    local mode_name = is_mode("debug") and "debug" or "release"
    
    -- 构建生成目录路径
    return path.join(os.projectdir(), "build", os_name, arch_name, mode_name)
end
function find_my_program(name, sdkdir, use_next)
    import("lib.detect.find_file")
    import("lib.detect.find_program")
    import("lib.detect.find_tool")

    local sdkdir = sdkdir or path.join(os.projectdir(), "tools")
    local exedir = find_exe_dir()
    local tool = find_tool(name, {pathes = {sdkdir, exedir, "/usr/local/bin"}})
    local prog = tool and tool.program or find_program(name, {pathes = {sdkdir, exedir, "/usr/local/bin"}})
    prog = prog or find_file(name, {sdkdir, exedir})
    if (prog == nil) then
        if os.host() ~= "windows" then
            local outdata, errdata = os.iorun("which " .. name)
            if (errdata ~= nil or errdata ~= "") then
                prog = string.gsub(outdata, "%s+", "")
            end
        else
            prog = find_file(name .. ".exe", {sdkdir})
        end
    end
    if (prog == nil) then
        if not use_next then
            return find_my_program(name, path.join(sdkdir, name), true)
        end
        print(name .. " not found! under " .. sdkdir, exedir)
        return
    end
    return {program = prog, sdkdir = sdkdir}
end
