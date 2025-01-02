import("core.project.depend")
function cmd_compile(target, genfile, file)
    import("core.project.project")
    local name = target:name()
    local pub_deps = target:values("module.public_dependencies")
    local cpp_content = "inline void __" .. name .. "__module::InitMetaData(void){\n"
    cpp_content = cpp_content.."\tmInfo.name = \"" .. name.."\";\n"
    cpp_content = cpp_content.."\tmInfo.dependencies =  {\n "
    for _,dep in ipairs(pub_deps) do 
        local deptarget = project.target(dep)
        if deptarget then
            local kind = deptarget:kind()
            local version = deptarget:version() or "0.0"
            cpp_content = cpp_content.."\t\t{\"" .. dep .. "\", \""..version.."\", \""..kind.."\" },\n"
        end
    end
    cpp_content = cpp_content.sub(cpp_content, 1, -3)
    cpp_content = cpp_content.."\n\t};\n};"
    print("cmd_compile plugin ", genfile)
    io.writefile(genfile, cpp_content)
end
function add_gen_dir(target)
    local sourcedir = path.join(target:configdir(), ".gens", ".inl")
    if not os.isdir(sourcedir) then
        os.mkdir(sourcedir)
    end
    target:add("includedirs", sourcedir, {public = true})
    sourcedir = path.join(os.projectdir(), sourcedir, "." .. target:name())
    if not os.isdir(sourcedir) then
        os.mkdir(sourcedir)
    end
    return sourcedir
end
function main(target, file)
    if true then 
        return
    end
    local sourcedir = add_gen_dir(target)
    local genfile = path.join(sourcedir, target:name() .. ".plugin.inl")
    local dependfile = target:dependfile(genfile)
    local sourcefile = string.lower(path.join(target:scriptdir(), file))
    depend.on_changed(
        function()
            cmd_compile(target, genfile, file)
        end,
        {dependfile = dependfile, files = sourcefile}
    )
end