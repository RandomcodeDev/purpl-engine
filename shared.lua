function fix_target(target)
    if is_plat("gdk", "gdkx") then
        target:set("prefixname", "")
        if target:kind() == "binary" then
            target:set("extension", ".exe")
        elseif target:kind() == "static" then
            target:set("extension", ".lib")
        elseif target:kind() == "shared" then
            target:set("extension", ".dll")
        end
    elseif is_plat("switch") then
        if target:kind() == "binary" then
            target:set("prefixname", "")
            target:set("extension", ".nss")
        elseif target:kind() == "static" then
            target:set("prefixname", "lib")
            target:set("extension", ".a")
        elseif target:kind() == "shared" then
            target:set("prefixname", "lib")
            target:set("extension", ".nrs")
        end
    else
        -- Of course POSIX or GNU or whoever gets to have "libutil.a" be a reserved name
        -- Other systems don't need this, since they have less common default library names
        if target:kind() == "static" then
            target:set("suffixname", "-purpl")
        end
    end
end

function setup_shared(root)
    set_version("0.0.0")

    set_warnings("everything")

    add_defines("_CRT_SECURE_NO_WARNINGS")
    add_defines("_GNU_SOURCE")

    set_languages("gnu11", "cxx23")

    if is_plat("windows") then
        add_defines("PURPL_WIN32")
    elseif is_plat("gdk") then
        add_defines("PURPL_WIN32", "PURPL_GDK")
    elseif is_plat("gdkx") then
        add_defines("PURPL_WIN32", "PURPL_GDK", "PURPL_GDKX")
    elseif is_plat("linux") then
        add_defines("PURPL_LINUX", "PURPL_UNIX")
    elseif is_plat("freebsd") then
        add_defines("PURPL_FREEBSD", "PURPL_UNIX")
    elseif is_plat("switch") then
        add_defines("PURPL_SWITCH", "PURPL_UNIX")
    end

    if is_mode("debug") then
        add_defines("PURPL_DEBUG")
        add_defines('PURPL_BUILD_TYPE="Debug"')
    elseif is_mode("release") then
        add_defines("PURPL_RELEASE")
        add_defines('PURPL_BUILD_TYPE="Release"')
    end

    add_includedirs(path.join(root, "deps/zstd/lib"))

    if is_plat("linux", "freebsd") then
        add_requires("glfw")
    end

    if is_plat("gdk", "linux", "freebsd", "switch") then
        add_defines("PURPL_VULKAN")

        add_includedirs(
            path.join(root, "deps/volk"),
            path.join(root, "deps/Vulkan-Headers/include"),
            path.join(root, "deps/VulkanMemoryAllocator/include")
        )

        if is_plat("gdk") then
            add_defines("VK_USE_PLATFORM_WIN32_KHR")
        elseif is_plat("linux", "freebsd") then
            add_defines("VK_USE_PLATFORM_WAYLAND_KHR", "VK_USE_PLATFORM_XCB_KHR")
        elseif is_plat("switch") then
            add_defines("VK_USE_PLATFORM_VI_NN")
        end
    end

    if is_plat("windows", "gdk", "gdkx") then
        add_cxflags("-wd4820", "-wd4255", "-wd4464", "-wd4668")
    end

    add_includedirs(
        root,
        path.join(root, "deps"),
        --path.join(root, "deps/cimgui"),
        --path.join(root, "deps/cimgui/generator/output"),
        --path.join(root, "deps/cimgui/imgui"),
        --path.join(root, "deps/imgui_backends"),
        path.join(root, "deps/cglm/include"),
        path.join(root, "deps/cjson"),
        path.join(root, "deps/flecs")
    )

    target("cjson")
        set_kind("static")
        add_headerfiles(path.join(root, "deps/cjson/cJSON.h"))
        add_files(path.join(root, "deps/cjson/cJSON.c"))
        on_load(fix_target)

    target("stb")
        set_kind("static")
        add_files(path.join(root, "deps/stb.c"))
        on_load(fix_target)

    target("zstd")
        set_kind("static")
        add_headerfiles(path.join(root, "deps/zstd/lib/**/*.h"))
        add_files(path.join(root, "deps/zstd/lib/**/*.c"))
        if is_plat("linux", "freebsd") then
            add_files(path.join(root, "deps/zstd/lib/**/*.S"))
        end
        on_load(fix_target)

    target("common")
        set_kind("static")
        add_headerfiles(path.join(root, "common/*.h"))
        add_files(path.join(root, "common/*.c"))
        add_deps("platform", "stb")
        on_load(fix_target)

    target("platform")
        set_kind("static")
        add_headerfiles(path.join(root, "platform/*.h"))

        if is_plat("gdk", "gdkx", "windows") then
            add_files(path.join(root, "platform/win32/platform.c"), path.join(root, "platform/win32/video.c"))
            add_links("advapi32", "kernel32", "shell32", "user32")
            if not is_plat("gdkx") then
                add_links("dbghelp")
            end
        elseif is_plat("linux", "freebsd") then
            add_files(
                path.join(root, "platform/unix/platform.c"),
                path.join(root, "platform/unix/video.c")
            )
            add_packages("glfw")
        elseif is_plat("switch") then
            add_files(path.join(root, "../platform/switch/platform.cpp"), path.join(root, "../platform/switch/video.cpp"))
            add_switch_links()
        end
        on_load(fix_target)

    target("util")
        set_kind("static")
        add_headerfiles(path.join(root, "util/*.h"))
        add_files(path.join(root, "util/*.c"))
        add_deps("cjson", "common", "zstd")
        on_load(fix_target)
end
