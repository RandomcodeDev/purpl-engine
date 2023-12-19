add_rules("mode.debug", "mode.release", "plugin.vsxmake.autoupdate")

set_project("purpl-engine")

set_allowedplats("gdk", "gdkx", "linux", "freebsd", "switch")
set_allowedarchs("gdk|x64", "gdkx|x64", "switch|arm64")

if os.isfile("../platform/switch/toolchain.lua") then
    includes("../platform/switch/toolchain.lua")
    if is_plat("switch") then
        add_switch_settings()
    end
else
    function add_switch_links() end
    function add_switch_vulkan_links() end
    function switch_postbuild(target) end
    function add_switch_rendersystem() end
end

directx = is_plat("gdk", "gdkx")
vulkan = is_plat("gdk", "linux", "freebsd")

includes("shared.lua")
setup_shared("$(scriptdir)", vulkan)

--target("imgui")
--    set_kind("static")
--    add_headerfiles("deps/cimgui/*.h", "deps/cimgui/imgui/*.h")
--    add_files("deps/cimgui/*.cpp", "deps/cimgui/imgui/*.cpp")
--    add_headerfiles("deps/imgui_backends/imgui_impl_purpl.h")
--    add_files("deps/imgui_backends/imgui_impl_purpl.cpp")
--    if vulkan then
--        add_headerfiles("deps/imgui_backends/imgui_impl_vulkan.h")
--        add_files("deps/imgui_backends/imgui_impl_vulkan.cpp")
--    end
--
--    add_defines("CIMGUI_NO_EXPORT")
--
--    on_load(fix_target)

target("flecs")
    set_kind("static")
    add_defines("FLECS_STATIC")
    add_headerfiles("deps/flecs/flecs.h")
    add_files("deps/flecs/flecs.c")

    on_load(fix_target)

target("engine")
    set_kind("static")
    add_headerfiles("engine/*.h")
    add_files("engine/*.c")
    add_deps("common", "cjson", "flecs", "platform", "rendersystem", "util")

    on_load(fix_target)

target("rendersystem")
    set_kind("static")
    add_headerfiles("engine/rendersystem/*.h")
    add_files("engine/rendersystem/*.c")

    add_deps("util")

    if directx then
        add_headerfiles("engine/rendersystem/directx/dx.h")
        add_files("engine/rendersystem/directx/dx.cpp")
        add_defines("PURPL_DIRECTX")
    end

    if vulkan then
        add_includedirs("deps/volk", "deps/Vulkan-Headers/include", "deps/VulkanMemoryAllocator/include")
        add_headerfiles("deps/VulkanMemoryAllocator/include/vk_mem_alloc.h", "engine/rendersystem/vulkan/*.h")
        add_files("engine/rendersystem/vulkan/*.c", "engine/rendersystem/vulkan/*.cpp")

        add_files("deps/volk/volk.c")
    end

    add_switch_rendersystem()

    on_load(fix_target)

target("purpl")
    set_kind("binary")
    add_headerfiles("purpl/*.h")
    add_files("purpl/*.c")
    add_deps("common", "engine", "platform", "util")
    set_default(true)

    if is_plat("gdk", "gdkx") then
        add_files("platform/win32/launcher.c", "platform/win32/purpl.rc")
    elseif is_plat("linux", "freebsd") then
        add_files("platform/unix/launcher.c")
    elseif is_plat("switch") then
        add_files("../platform/switch/launcher.cpp")
        after_build(switch_postbuild)
    end

    on_load(fix_target)
    before_build(function (target)
        if not os.exists(path.join(target:targetdir(), "assets")) then
            os.ln(path.absolute("assets/out"), path.join(target:targetdir(), "assets"))
        end
    end)
