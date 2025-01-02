import("core.project.project")
function main(target)
    for _, pkg in pairs(target:pkgs()) do
        if pkg:requireconf("configs","copy") then
            local link = pkg:get("links")
            local targetdir = target:targetdir()
            link = link[1] or link
            if is_mode("debug") and not os.isdir(targetdir) then 
                os.mkdir(targetdir)
            end
            if link and os.isdir(targetdir) and not os.isfile(path.join(targetdir, link .. ".lib")) then 
                local linkdirs = pkg:get("linkdirs")
                os.trycp(linkdirs.."/*", targetdir)
                print("copy",linkdirs, targetdir)
            end
        end
    end
end