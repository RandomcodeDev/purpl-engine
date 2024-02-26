add_rules(
    "mode.debug",
    "mode.release",
    "plugin.vsxmake.autoupdate"
)

set_project("purpl-engine-tools")

add_requires("assimp")

includes("../shared.lua")
setup_shared("$(scriptdir)/..", false)

target("meshtool")
    set_kind("binary")
    add_files("tools/meshtool.c")
    add_deps("common", "platform", "util")
    add_packages("assimp")

target("texturetool")
    set_kind("binary")
    add_files("tools/texturetool.c")
    add_deps("common", "platform", "util")
