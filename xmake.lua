add_rules(
    "mode.debug",
    "mode.release",
    "plugin.vsxmake.autoupdate"
)

if os.isfile(path.join("..", "platform", "switch", "switch.lua")) then
    includes(path.join("..", "platform", "switch", "switch.lua"))
else
    function add_switch_links() end
    function add_switch_vulkan_links() end
    function switch_postbuild(target) end
    function add_switch_renderapi() end
end

set_project("purpl-engine")
set_version("0.0.0", {build = "%Y%m%d%H%M"})

set_allowedplats("gdk", "gdkx", "windows", "linux", "freebsd", "switch")
set_allowedarchs("gdk|x64", "gdkx|x64", "windows|x86", "switch|arm64")

local directx = is_plat("gdk", "gdkx", "windows")
local vulkan = is_plat("gdk", "windows", "linux", "freebsd", "switch")

local discord = is_plat("gdk", "gdkx", "windows", "macos", "linux", "freebsd")
local use_mimalloc = not is_plat("switch")

add_defines("PURPL_ENGINE")

includes(path.join("xmake", "shared.lua"))
setup_shared("$(scriptdir)", directx, vulkan)

includes(path.join("support", "support.lua"))
setup_support("support", path.join("support", "deps"), use_mimalloc, vulkan, true)

if is_plat("switch") then
    add_switch_settings()
end

if discord then
    target("discord")
        set_kind("static")
        add_headerfiles(path.join("deps", "discord-rpc", "include", "*.h"), path.join("deps", "discord-rpc", "src", "*.h"))
        add_files(
            path.join("deps", "discord-rpc", "src", "discord_rpc.cpp"),
            path.join("deps", "discord-rpc", "src", "rpc_connection.cpp"),
            path.join("deps", "discord-rpc", "src", "serialization.cpp")
        )

        if is_plat("gdk", "gdkx") then
            add_files(path.join("deps", "discord-rpc", "src", "*_win.cpp"))
        elseif is_plat("linux", "freebsd") then
            add_files(path.join("deps", "discord-rpc", "src", "*_linux.cpp"), path.join("deps", "discord-rpc", "src", "*_unix.cpp"))
        end

        set_group("Engine")

        set_warnings("none")
        on_load(fix_target)
    target_end()

    add_defines("PURPL_DISCORD")
    add_includedirs(path.join("deps", "discord-rpc", "include"), path.join("deps", "rapidjson", "include"))
end

target("flecs")
    set_kind("static")
    add_defines("FLECS_STATIC")
    add_headerfiles(path.join("deps", "flecs", "flecs.h"))
    add_files(path.join("deps", "flecs", "flecs.c"))
    set_warnings("none")
    set_group("External")

    on_load(fix_target)
target_end()

includes(path.join("xmake", "mujoco.lua"))

if vulkan then
    target("render-vk")
        set_kind("static")
        add_headerfiles(
            path.join("deps", "VulkanMemoryAllocator", "include", "vk_mem_alloc.h"),
            path.join("engine", "render", "vk", "*.h")
        )
        add_files(
            path.join("deps", "volk", "volk.c"),
            path.join("engine", "render", "vk", "*.c"),
            path.join("engine", "render", "vk", "*.cpp")
        )

        add_deps("util")

        if is_plat("switch") then
            add_switch_vulkan_links()
        end

        set_group("Engine/Render System")

        on_load(fix_target)
    target_end()
end

if directx then
    target("render-dx12")
        set_kind("static")
        add_headerfiles("engine", "render", "dx12", "*.h")
        add_files(
            path.join("deps", "DirectX-Headers", "src", "dxguids.cpp"),
            path.join("deps", "D3D12MemoryAllocator", "src", "Common.cpp"),
            path.join("deps", "D3D12MemoryAllocator", "src", "D3D12MemAlloc.cpp"),
            --path.join("engine", "render", "dx12", "*.c"),
            path.join("engine", "render", "dx12", "*.cpp")
        )

        if is_plat("gdk") then
            add_links("d3d12.lib", "dxgi.lib")
        elseif is_plat("gdkx") then
            add_links("d3d12_xs.lib", "dxgi_xs.lib")
        end

        set_group("Engine/Render System")

        on_load(fix_target)
    target_end()
end

if is_plat("switch") then
    add_switch_renderapi()
end

target("render-swrast")
    set_kind("static")
    add_headerfiles(path.join("engine", "render", "swrast", "*.h"))
    add_files(path.join("engine", "render", "swrast", "*.c"))

    set_group("Engine/Render System")

    on_load(fix_target)
target_end()

target("render")
    set_kind("static")
    add_headerfiles(path.join("engine", "render", "*.h"))
    add_files(path.join("engine", "render", "*.c"))

    add_deps("util")

    if directx then
        add_deps("render-dx12")
    end
    if vulkan then
        add_deps("render-vk")
    end
    add_deps("render-swrast")

    set_group("Engine/Render System")

    on_load(fix_target)
target_end()

target("engine")
    set_kind("static")
    add_headerfiles(path.join("engine", "*.h"))
    add_files(path.join("engine", "*.c"))
    add_deps("common", "cjson", "flecs", "mujoco", "platform", "render", "util")
    if discord then
        add_headerfiles(path.join("engine", "discord", "*.h"))
        add_files(path.join("engine", "discord", "*.c"))
        add_deps("discord")
    end

    set_group("Engine")

    on_load(fix_target)
target_end()

target("purpl")
    set_kind("binary")
    -- header files in this case are just anything that doesn't participate in the build
    add_headerfiles(
        path.join("assets", "*"),
        path.join("assets", "shaders", "*"),
        path.join("purpl", "*.h")
    )
    add_files(path.join("purpl", "*.c"))
    add_deps("common", "engine", "platform", "util")
    set_default(true)

    support_executable("support")

    if is_plat("gdk", "gdkx", "windows") then
        if not is_plat("windows") then
            add_headerfiles(path.join("platform", "gdk", "MicrosoftGameConfig.mgc"))
            add_links("xgameruntime.lib")
            after_build(function (target)
                os.cp(path.absolute(path.join("gdk", "MicrosoftGameConfig.mgc")), path.join(target:targetdir(), "MicrosoftGame.Config"))
            end)
        end
    elseif is_plat("switch") then
        add_headerfiles(path.join("..", "platform", "switch", "switch.lua"))
        add_files(path.join("..", "platform", "switch", "launcher.cpp"))
        after_build(switch_postbuild)
    end

    on_load(fix_target)
    before_build(function (target)
        if not os.exists(path.join(target:targetdir(), "assets")) then
            os.ln(path.absolute(path.join("assets", "out")), path.join(target:targetdir(), "assets"))
        end

        if is_plat("gdk", "gdkx") and not os.exists(path.join(target:targetdir(), "GdkAssets")) then
            os.ln(path.absolute(path.join("gdk", "GdkAssets")), path.join(target:targetdir(), "GdkAssets"))
        end
    end)
target_end()
