function fix_target(target)
    target:add("options", "mimalloc")
    target:add("options", "verbose")

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
    elseif not is_plat("windows") then
        -- Of course POSIX or GNU or whoever gets to have "libutil.a" be a reserved name
        -- Other systems don't need this, since they have less common default library names
        if target:kind() == "static" then
            target:set("suffixname", "-purpl")
        end
    end
end

default_mimalloc = not is_plat("switch")

function setup_shared(root, directx, vulkan)
    set_version("0.0.0", {build = "%Y%m%d%H%M"})

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
        add_defines("CmnFreeBSD", "PURPL_UNIX")
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

    if is_plat("linux", "freebsd") then
        add_requires("glfw")
    end

    add_includedirs(
        root,
        path.join(root, "deps"),
        path.join(root, "deps/cglm/include"),
        path.join(root, "deps/cjson"),
        path.join(root, "deps/flecs"),
        path.join(root, "deps/mimalloc/include"),
        path.join(root, "deps/zstd/lib"),
        path.absolute("$(buildir)/config")
    )

    if directx then
        add_defines("PURPL_DIRECTX")

        add_includedirs(
            path.join(root, "deps/D3D12MemoryAllocator/include"),
            path.join(root, "deps/DirectX-Headers/include"),
            path.join(root, "deps/DirectX-Headers/include/directx"),
            path.join(root, "deps/DirectX-Headers/include/dxguids")
        )
    end

    if vulkan then
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
        -- These are just noise, like alignment and stuff
        add_cxflags("-wd4820", "-wd4255", "-wd4464", "-wd4668", "-wd5045", {force = true})

        add_linkdirs(
            path.join(os.getenv("GRDKLatest"), "GameKit/Lib/amd64")
        )
    end

    if is_plat("linux", "freebsd", "switch") then
        -- Old style casts are to match stylistically, and C++98 is defunct as hell
        -- Also, CmnFree sets the variable to NULL to reduce misuse, and uses a
        -- block to do that, but having a semi after looks normal, even if it's
        -- technically superfluous
        add_cxflags("-Wno-gnu-line-marker", "-Wno-c++98-compat", "-Wno-c++98-compat-pedantic", "-Wno-old-style-cast", "-Wno-extra-semi-stmt", {force = true})
    end

    target("cjson")
        set_kind("static")
        add_headerfiles(path.join(root, "deps/cjson/cJSON.h"))
        add_files(path.join(root, "deps/cjson/cJSON.c"))
        on_load(fix_target)

    if default_mimalloc then
        target("mimalloc")
            set_kind("static")
            add_files(
                path.join(root, "deps/mimalloc/src/alloc.c"),
                path.join(root, "deps/mimalloc/src/alloc-aligned.c"),
                path.join(root, "deps/mimalloc/src/alloc-posix.c"),
                path.join(root, "deps/mimalloc/src/arena.c"),
                path.join(root, "deps/mimalloc/src/bitmap.c"),
                path.join(root, "deps/mimalloc/src/heap.c"),
                path.join(root, "deps/mimalloc/src/init.c"),
                path.join(root, "deps/mimalloc/src/options.c"),
                path.join(root, "deps/mimalloc/src/os.c"),
                path.join(root, "deps/mimalloc/src/page.c"),
                path.join(root, "deps/mimalloc/src/random.c"),
                path.join(root, "deps/mimalloc/src/segment.c"),
                path.join(root, "deps/mimalloc/src/segment-map.c"),
                path.join(root, "deps/mimalloc/src/stats.c"),
                path.join(root, "deps/mimalloc/src/prim/prim.c")
            )
            if is_plat("windows", "gdk", "gdkx") then
                add_files(path.join(root, "deps/mimalloc/src/prim/windows/*.c"))
            elseif is_plat("macos") then
                add_files(path.join(root, "deps/mimalloc/src/prim/osx/*.c"))
            else
                add_files(path.join(root, "deps/mimalloc/src/prim/unix/*.c"))
            end
            add_forceincludes("stdio.h")
            on_load(fix_target)
    end

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

    option("verbose")
        set_default(false)
        set_description("Enable verbose logging")
        add_defines("PURPL_VERBOSE")

    option("mimalloc")
        set_default(default_mimalloc)
        set_description("Enable the use of mimalloc")
        add_defines("PURPL_USE_MIMALLOC")
        add_links("mimalloc")

    target("common")
        set_kind("static")
        set_configdir("$(buildir)/config")
        add_configfiles(path.join(root, "purpl/config.h.in"))
        add_headerfiles(path.join(root, "common/*.h"))
        add_files(path.join(root, "common/*.c"))
        add_deps("platform", "stb")
        on_load(fix_target)

    target("platform")
        set_kind("static")
        add_headerfiles(path.join(root, "platform/*.h"))

        if is_plat("gdk", "gdkx", "windows") then
            add_files(
                path.join(root, "platform/win32/async.c"),
                path.join(root, "platform/win32/platform.c"),
                path.join(root, "platform/win32/video.c")
            )
            add_links("advapi32", "kernel32", "shell32", "user32")
            if not is_plat("gdkx") then
                add_links("dbghelp")
            end
        elseif is_plat("linux", "freebsd") then
            add_files(
                path.join(root, "platform/unix/async.c"),
                path.join(root, "platform/unix/platform.c"),
                path.join(root, "platform/unix/video.c")
            )
            add_packages("glfw")
        elseif is_plat("switch") then
            add_files(
                path.join(root, "../platform/switch/async.cpp"),
                path.join(root, "../platform/switch/platform.cpp"),
                path.join(root, "../platform/switch/video.cpp")
            )
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
