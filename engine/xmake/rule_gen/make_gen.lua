import("core.project.depend")
function cmd_compile(genfile, sourcefile, template, macro, define)
    import("find_sdk")
    local meta = find_sdk.find_my_program("refl")
    template = template or path.join(meta.sdkdir, "template")
    if not macro then --优先使用库定义
        macro = path.join(os.projectdir(), "engine/modules/engine/zlib/include/refl/macro.h")
        if not os.exists(macro) then
            macro = path.join(os.curdir(), "macro.h")
        end
    end
    argv = {"build", sourcefile, "-o", genfile, "-t", template, "-m", macro}
    if define then
        table.insert(argv, "-d")
        table.insert(argv, define)
    end
    print("cmd_meta_compile", genfile)
    os.execv(meta.program, argv)
    return argv
end

function _listen_gen_file(target, batch, template, macro, define)
    genfile, sourcefile = batch[1], batch[2]
    sourcefile = string.lower(sourcefile)
    local dependfile = target:dependfile(genfile)
    if false then 
        cmd_compile(batch[1], batch[2], template, macro, define)
        return
    end
    depend.on_changed(
        function()
            cmd_compile(batch[1], batch[2], template, macro, define)
        end,
        {dependfile = dependfile, files = sourcefile}
    )
end    
function gen(target)
    if is_mode("release") then return end
    local gen_batch = target:data("codegen.batch")
    if not gen_batch then
        return
    end
    local template = target:extraconf("rules", "c++.codegen", "template")
    local macro = target:extraconf("rules", "c++.codegen", "macro")
    local define = target:extraconf("rules", "c++.codegen", "define")
    for _, batch in ipairs(gen_batch) do
        if batch[2] then
            _listen_gen_file(target, batch, template, macro, define)
        end
    end
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
function main(target, headerfiles)
    local sourcedir = add_gen_dir(target)
    local gen_batch = {}
    for idx, headerfile in pairs(headerfiles) do
        -- batch
        local sourcefile = path.join(sourcedir, path.basename(headerfile) .. "_gen.inl")
        table.insert(gen_batch, {sourcefile, headerfile})
    end
    -- save unit batch
    target:data_set("codegen.batch", gen_batch)
end
