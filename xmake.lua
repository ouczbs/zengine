--add_repositories("local_repo F:\\xmake.repo")
add_repositories("local_repo https://github.com/ouczbs/xmake.repo.git")
add_rules("mode.debug", "mode.release")
set_version("1.0.1", {soname = true})
set_arch("x64")
set_languages("cxx20")
set_project("zengine")
set_toolchains("clang")
set_runtimes("MD","c++_shared")
add_cxxflags("-stdlib=libc++", "-Wno-parentheses-equality")
add_ldflags("-stdlib=libc++")
includes("engine")
includes("game/*/xmake.lua")
--xmake project -k vsxmake2022 -a x64
--xmake project -k vsxmake2022 -m "debug;release"
--xmake build -vD -y  -P .  "zworld-editor"
--xrepo env -b emmylua_debugger -- xmake project -k vsxmake2022 -m "debug;release"