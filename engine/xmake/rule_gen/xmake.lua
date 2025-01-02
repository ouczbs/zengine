rule("c++.codegen")
    set_extensions(".inl")
    after_load(function (target)
        import("make_gen")
        local headerfiles = {}
        local files = target:extraconf("rules", "c++.codegen", "files")
        for _, file in ipairs(files) do
            local p = path.join(target:scriptdir(), file)
            for __, filepath in ipairs(os.files(p)) do
                table.insert(headerfiles, filepath)
            end
        end
        make_gen(target, headerfiles)
    end)
    on_config(function (target)
        import("make_gen").gen(target)
    end)