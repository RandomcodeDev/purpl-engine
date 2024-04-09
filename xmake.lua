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

local switch_title_id = "0100694203488000"

local directx = is_plat("gdk", "gdkx", "windows")
local vulkan = is_plat("gdk", "windows", "linux", "freebsd", "switch")

local discord = is_plat("gdk", "gdkx", "windows", "macos", "linux", "freebsd")
local use_mimalloc = not is_plat("switch")

add_defines("PURPL_ENGINE")

includes(path.join("xmake", "shared.lua"))
setup_shared("$(scriptdir)", directx, vulkan)

includes(path.join("support", "support.lua"))
setup_support("support", path.join("support", "deps"), use_mimalloc, vulkan, false, true, "purpl/config.h.in", switch_title_id)

if discord then
    target("discord")
        set_kind("static")
        add_headerfiles(path.join("deps", "discord-rpc", "include", "*.h"), path.join("deps", "discord-rpc", "src", "*.h"))
        add_files(
            path.join("deps", "discord-rpc", "src", "discord_rpc.cpp"),
            path.join("deps", "discord-rpc", "src", "rpc_connection.cpp"),
            path.join("deps", "discord-rpc", "src", "serialization.cpp")
        )

        if is_plat("gdk", "gdkx", "windows") then
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

    if get_config("toolchain") == "mingw" then
        add_links("ws2_32")
    end

    on_load(fix_target)
target_end()

target("imgui")
    set_kind("static")
    add_defines("CIMGUI_NO_EXPORT", "IMGUI_STATIC")
    add_headerfiles(path.join("deps", "cimgui", "*.h"), path.join("deps", "cimgui", "imgui", "*.h"))
    add_files(path.join("deps", "cimgui", "*.cpp"), path.join("deps", "cimgui", "imgui", "*.cpp"))
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
        add_headerfiles(path.join("engine", "render", "dx12", "*.h"))
        add_files(
            path.join("deps", "D3D12MemoryAllocator", "src", "Common.cpp"),
            path.join("deps", "D3D12MemoryAllocator", "src", "D3D12MemAlloc.cpp"),
            --path.join("engine", "render", "dx12", "*.c"),
            path.join("engine", "render", "dx12", "*.cpp")
        )

        if get_config("toolchain") == "mingw" then
            add_defines("__REQUIRED_RPCNDR_H_VERSION__=475")
            add_forceincludes("math.h")
        end

        if get_config("toolchain") == "mingw" then
            add_links("d3d12", "dxgi")
        end

        if is_plat("gdk", "windows") then
            if get_config("toolchain") == "mingw" then
                add_links("d3d12", "dxgi")
            else
                add_links("d3d12.lib", "dxgi.lib")
            end
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

    set_group("Engine/Render System")

    on_load(fix_target)
target_end()

target("engine")
    set_kind("static")
    add_headerfiles(path.join("engine", "*.h"))
    add_files(path.join("engine", "*.c"))
    add_deps("common", "cjson", "flecs", "imgui", "mujoco", "platform", "render", "util")
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

    if is_plat("gdk", "gdkx") then
        add_headerfiles(path.join("gdk", "*.mgc"))
        if get_config("toolchain") == "mingw" then
            add_links("xgameruntime")
        else
            add_links("xgameruntime.lib")
        end
    end

    on_load(fix_target)
    before_build(function (target)
        target:set("support_data", {
            "Purpl",
            "Randomcode Developers",
            switch_title_id
        })

        if not os.exists(path.join(target:targetdir(), "assets")) then
            os.ln(path.absolute(path.join("assets", "out")), path.join(target:targetdir(), "assets"))
        end

        if is_plat("gdk", "gdkx") then
            os.cp(path.absolute(path.join("gdk", "MicrosoftGameConfig.$(plat).mgc")), path.join(target:targetdir(), "MicrosoftGame.Config"))
            if not os.exists(path.join(target:targetdir(), "GdkAssets")) then
                os.ln(path.absolute(path.join("gdk", "GdkAssets")), path.join(target:targetdir(), "GdkAssets"))
            end
        end
    end)
target_end()
