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
        -- Other systems don't need this, since they don't pull shit like this
        if target:kind() == "static" then
            target:set("suffixname", "-purpl")
        end
    end
end

use_mimalloc = not is_plat("switch")

function setup_shared(root, directx, vulkan)
    set_version("0.0.0", {build = "%Y%m%d%H%M"})

    set_warnings("everything")

    add_defines("_CRT_SECURE_NO_WARNINGS")
    add_defines("_GNU_SOURCE")

    set_languages("gnu11", "cxx23")
    set_exceptions("cxx")

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
        path.join(root, "deps/mujoco/include"),
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
        add_cxflags("-Qspectre", {force = true})
        -- all of these are either external or inconsequential
        add_cxflags(
            "-wd4820", -- padded
            "-wd4365", -- signed/unsigned mismatch (generally doesn't matter)
            "-wd4255", -- () to (void)
            "-wd4244", -- int to float
            "-wd4464", -- relative include path has ./.., this is fine because the build is setup right
            "-wd4061", -- enum not explicitly handled by a case label
            "-wd4062",
            "-wd4324", -- padded for alignment specifier
            "-wd4005", -- macro redefinition
            "-wd4668", -- x is not defined as a preprocessor macro, replacing with 0 for #if
            "-wd4113", -- Spectre mitigation
            "-wd5045",
            "-wd4191", -- casting function pointer (used for InitializeMainThread, it doesn't call the pointer)
            "-wd5029", -- nonstandard extension: alignment attributes don't apply to functions
        {force = true})
        add_cxxflags(
            "-wd5204", -- virtual function something something
            "-wd5027", -- move assignment operator was defined as deleted
            "-wd4626", -- assignment operator was defined as deleted
            "-wd4623", -- default constructor was defined as deleted
            "-wd4625", -- copy constructor was defined as deleted
        {force = true})
        add_linkdirs(
            path.join(os.getenv("GRDKLatest"), "GameKit/Lib/amd64")
        )
    end

    if is_plat("linux", "freebsd", "switch") then
        add_cxflags(
            "-Wno-gnu-line-marker",
            "-Wno-gnu-zero-variadic-macro-arguments",
            "-Wno-extra-semi-stmt",
        {force = true})
        add_cxxflags(
            "-Wno-c++98-compat",
            "-Wno-c++98-compat-pedantic",
            "-Wno-old-style-cast",
        {force = true})
    end

    target("cjson")
        set_kind("static")
        add_headerfiles(path.join(root, "deps/cjson/cJSON.h"))
        add_files(path.join(root, "deps/cjson/cJSON.c"))
        set_warnings("none")
        set_group("External")
        on_load(fix_target)

    if use_mimalloc then
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

            if is_mode("debug") then
                add_defines("MI_DEBUG=3", "MI_SECURE=4")
            end

            set_warnings("none")

            set_group("External")

            add_forceincludes("stdio.h")
            on_load(fix_target)
    end

    target("stb")
        set_kind("static")
        add_files(path.join(root, "deps/stb.c"))
        set_warnings("none")
        set_group("External")
        on_load(fix_target)

    target("zstd")
        set_kind("static")
        add_headerfiles(path.join(root, "deps/zstd/lib/**/*.h"))
        add_files(path.join(root, "deps/zstd/lib/**/*.c"))
        if is_plat("linux", "freebsd") then
            add_files(path.join(root, "deps/zstd/lib/**/*.S"))
        end
        set_warnings("none")
        set_group("External")
        on_load(fix_target)

    option("verbose")
        set_default(false)
        set_description("Enable verbose logging")
        add_defines("PURPL_VERBOSE")

    target("common")
        set_kind("static")

        set_configdir("$(buildir)/config")
        set_configvar("USE_MIMALLOC", use_mimalloc and 1 or 0)
        add_configfiles(path.join(root, "purpl/config.h.in"))
        
        add_headerfiles(path.join(root, "common/*.h"), path.join(root, "purpl/*.h*"), path.join(root, "shared.lua"))
        add_files(path.join(root, "common/*.c"))

        set_group("Support")
        
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
            if is_arch("x86") then
                add_links("atomic")
            end
        elseif is_plat("switch") then
            add_files(
                path.join(root, "../platform/switch/async.cpp"),
                path.join(root, "../platform/switch/platform.cpp"),
                path.join(root, "../platform/switch/video.cpp")
            )
            add_switch_links()
        end

        if use_mimalloc then
            add_deps("mimalloc")
        end

        set_group("Support")

        on_load(fix_target)

    target("util")
        set_kind("static")
        add_headerfiles(path.join(root, "util/*.h"))
        add_files(path.join(root, "util/*.c"))
        add_deps("cjson", "common", "zstd")
        set_group("Support")
        on_load(fix_target)
end
