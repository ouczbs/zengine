rule("engine.tool")
    after_build(function (target)
        local tooldir = path.join(os.projectdir(), "tools", target:name())
        if not os.isdir(tooldir) then 
            os.mkdir(tooldir)
        end
        local exefile = target:targetfile()
        os.cp(exefile, path.join(tooldir, path.filename(exefile)))
    end)