add_rules(
    "mode.debug",
    "mode.release",
    "plugin.vsxmake.autoupdate"
)

set_project("purpl-engine")

set_allowedplats("gdk", "gdkx", "linux", "freebsd", "switch")
set_allowedarchs("gdk|x64", "gdkx|x64", "switch|arm64")

if os.isfile("../platform/switch/switch.lua") then
    includes("../platform/switch/switch.lua")
    if is_plat("switch") then
        add_switch_settings()
    end
else
    function add_switch_links() end
    function add_switch_vulkan_links() end
    function switch_postbuild(target) end
    function add_switch_renderapi() end
end

directx = is_plat("gdk", "gdkx")
discord = is_plat("gdk", "gdkx", "macos", "linux", "freebsd")
vulkan = is_plat("gdk", "linux", "freebsd", "switch")

includes("shared.lua")
setup_shared("$(scriptdir)", directx, vulkan)

if discord then
    target("discord")
        set_kind("static")
        add_headerfiles("deps/discord-rpc/include/*.h", "deps/discord-rpc/src/*.h")
        add_files(
            "deps/discord-rpc/src/discord_rpc.cpp",
            "deps/discord-rpc/src/rpc_connection.cpp",
            "deps/discord-rpc/src/serialization.cpp"
        )

        if is_plat("gdk", "gdkx") then
            add_files("deps/discord-rpc/src/*_win.cpp")
        elseif is_plat("linux", "freebsd") then
            add_files("deps/discord-rpc/src/*_linux.cpp", "deps/discord-rpc/src/*_unix.cpp")
        end

        set_warnings("none")
        on_load(fix_target)
    target_end()

    add_defines("PURPL_DISCORD")
    add_includedirs("deps/discord-rpc/include", "deps/rapidjson/include")

    set_group("Engine")
end

target("flecs")
    set_kind("static")
    add_defines("FLECS_STATIC")
    add_headerfiles("deps/flecs/flecs.h")
    add_files("deps/flecs/flecs.c")
    set_warnings("none")
    set_group("External")

    on_load(fix_target)

includes("mujoco.lua")

if vulkan then
    target("render-vk")
        set_kind("static")
        add_headerfiles("deps/VulkanMemoryAllocator/include/vk_mem_alloc.h", "engine/render/vk/*.h")
        add_files("deps/volk/volk.c", "engine/render/vk/*.c", "engine/render/vk/*.cpp")

        add_deps("util")

        if is_plat("switch") then
            add_switch_vulkan_links()
        end

        set_group("Engine/Render System")

        on_load(fix_target)
end

if directx then
    target("render-dx12")
        set_kind("static")
        add_headerfiles("engine/render/dx12/*.h")
        add_files(
            "deps/DirectX-Headers/src/dxguids.cpp",
            "deps/D3D12MemoryAllocator/src/Common.cpp",
            "deps/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp",
            --"engine/render/dx12/*.c",
            "engine/render/dx12/*.cpp"
        )

        if is_plat("gdk") then
            add_links("d3d12.lib", "dxgi.lib")
        elseif is_plat("gdkx") then
            add_links("d3d12_xs.lib", "dxgi_xs.lib")
        end

        set_group("Engine/Render System")

        on_load(fix_target)
end

add_switch_renderapi()

target("render")
    set_kind("static")
    add_headerfiles("engine/render/*.h")
    add_files("engine/render/*.c")

    add_deps("util")

    if directx then
        add_deps("render-dx12")
    end
    if vulkan then
        add_deps("render-vk")
    end

    set_group("Engine/Render System")

    on_load(fix_target)

target("engine")
    set_kind("static")
    add_headerfiles("engine/*.h")
    add_files("engine/*.c")
    add_deps("common", "cjson", "flecs", "mujoco", "platform", "render", "util")
    if discord then
        add_headerfiles("engine/discord/*.h")
        add_files("engine/discord/*.c")
        add_deps("discord")
    end

    set_group("Engine")

    on_load(fix_target)

target("purpl")
    set_kind("binary")
    -- header files in this case are just anything that doesn't participate in the build
    add_headerfiles(
        "assets/*",
        "assets/shaders/*",
        "purpl/*.h"
    )
    add_files("purpl/*.c")
    add_deps("common", "engine", "platform", "util")
    set_default(true)

    if is_plat("gdk", "gdkx") then
        add_files("platform/win32/launcher.c", "platform/win32/purpl.rc")
        add_links("xgameruntime.lib")
        after_build(function (target)
            if not os.exists(path.join(target:targetdir(), "MicrosoftGame.Config")) then
                os.ln(path.absolute("platform/gdk/MicrosoftGameConfig.mgc"), path.join(target:targetdir(), "MicrosoftGame.Config"))
            end
        end)
        if is_mode("debug") then
            add_ldflags("-subsystem:console")
        else
            add_ldflags("-subsystem:windows")
        end
    elseif is_plat("linux", "freebsd") then
        add_files("platform/unix/launcher.c")
    elseif is_plat("switch") then
        add_files("../platform/switch/launcher.cpp")
        after_build(switch_postbuild)
    end

    set_group("")

    on_load(fix_target)
    before_build(function (target)
        if not os.exists(path.join(target:targetdir(), "assets")) then
            os.ln(path.absolute("assets/out"), path.join(target:targetdir(), "assets"))
        end
    end)
