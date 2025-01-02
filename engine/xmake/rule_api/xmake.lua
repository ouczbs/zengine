rule("engine.api")
    on_config(function (target)
        import("rule_api")
        rule_api(target)
    end)
    on_load(function (target)
        import("package_api")
        package_api(target)
    end)