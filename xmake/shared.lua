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
    
    includes(path.join(root, "support", "support.lua"))
    setup_support(path.join(root, "support"), path.join(root, "support", "deps"), use_mimalloc, true, fix_target)

    add_includedirs(
        root,
        path.join(root, "deps"),
        path.join(root, "deps", "flecs"),
        path.join(root, "deps", "mujoco", "include")
    )

    if directx then
        add_defines("PURPL_DIRECTX")

        add_includedirs(
            path.join(root, "deps", "D3D12MemoryAllocator", "include"),
            path.join(root, "deps", "DirectX-Headers", "include"),
            path.join(root, "deps", "DirectX-Headers", "include", "directx"),
            path.join(root, "deps", "DirectX-Headers", "include", "dxguids")
        )
    end

    if vulkan then
        add_defines("PURPL_VULKAN")

        add_includedirs(
            path.join(root, "deps", "volk"),
            path.join(root, "deps", "Vulkan-Headers", "include"),
            path.join(root, "deps", "VulkanMemoryAllocator", "include")
        )

        if is_plat("gdk") then
            add_defines("VK_USE_PLATFORM_WIN32_KHR")
        elseif is_plat("linux", "freebsd") then
            add_defines("VK_USE_PLATFORM_WAYLAND_KHR", "VK_USE_PLATFORM_XCB_KHR")
        elseif is_plat("switch") then
            add_defines("VK_USE_PLATFORM_VI_NN")
        end
    end
end
