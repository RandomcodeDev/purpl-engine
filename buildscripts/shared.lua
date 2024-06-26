function setup_shared(root, directx, vulkan, opengl, swrast)
    add_includedirs(
        root,
        path.join(root, "deps")
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

    if opengl then
        add_defines("PURPL_OPENGL")
    end

    if swrast then
        add_defines("PURPL_SWRAST")
    end
end
