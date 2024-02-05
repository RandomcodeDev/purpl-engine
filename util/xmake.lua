add_rules("mode.debug", "mode.release")

set_project("purpl-engine-tools")

includes("../shared.lua")
setup_shared("$(scriptdir)/..", false)

-- TODO: remake using UFBX
--target("meshtool")
--    set_kind("binary")
--    add_files("tools/meshtool.c")
--    add_deps("common", "platform", "util")

target("texturetool")
    set_kind("binary")
    add_files("tools/texturetool.c")
    add_deps("common", "platform", "util")
