#include "vk.h"

PCSTR RequiredDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_ROBUSTNESS_2_EXTENSION_NAME};

VOID VlkEnumeratePhysicalDevices(VOID)
{
    UINT32 DeviceCount;
    VkPhysicalDevice *Devices = NULL;
    UINT32 UsableCount;
    VkResult Result;
    UINT32 i;
    UINT32 j;

    LogDebug("Enumerating devices");
    VULKAN_CHECK(vkEnumeratePhysicalDevices(VlkData.Instance, &DeviceCount, NULL));

    LogDebug("Required device extensions:");
    for (i = 0; i < PURPL_ARRAYSIZE(RequiredDeviceExtensions); i++)
    {
        LogDebug("\t%s", RequiredDeviceExtensions[i]);
    }

    LogDebug("Getting %d device handle(s)", DeviceCount);
    stbds_arrsetlen(Devices, DeviceCount);
    VULKAN_CHECK(vkEnumeratePhysicalDevices(VlkData.Instance, &DeviceCount, Devices));

    LogDebug("Initializing GPU info list");
    stbds_arrsetlen(VlkData.Gpus, DeviceCount);

    memset(VlkData.Gpus, 0, stbds_arrlenu(VlkData.Gpus) * sizeof(VULKAN_GPU_INFO));

    UsableCount = 0;
    for (i = 0; i < stbds_arrlenu(Devices); i++)
    {
        PVULKAN_GPU_INFO CurrentGpu = &VlkData.Gpus[i];
        CurrentGpu->Device = Devices[i];
        CurrentGpu->Usable = TRUE; // Assume this until it's disproven

        LogDebug("Getting information for device %u", i + 1);

        LogTrace("Getting queue family information");
        UINT32 QueueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(CurrentGpu->Device, &QueueCount, NULL);
        if (QueueCount < 1)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no queue families", i + 1);
            continue;
        }

        stbds_arrsetlen(CurrentGpu->QueueFamilyProperties, QueueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(CurrentGpu->Device, &QueueCount, CurrentGpu->QueueFamilyProperties);
        if (QueueCount < 1)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no queue families", i + 1);
            continue;
        }

        CurrentGpu->GraphicsFamilyIndex = UINT32_MAX;
        CurrentGpu->PresentFamilyIndex = UINT32_MAX;
        for (j = 0; j < stbds_arrlenu(CurrentGpu->QueueFamilyProperties); j++)
        {
            VkQueueFamilyProperties *Properties = &CurrentGpu->QueueFamilyProperties[j];

            if (Properties->queueCount < 1)
            {
                continue;
            }

            if (Properties->queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                CurrentGpu->GraphicsFamilyIndex = j;
            }

            LogTrace("Checking surface support for queue family %u", j);
            VkBool32 PresentSupported = FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(CurrentGpu->Device, j, VlkData.Surface, &PresentSupported);
            if (PresentSupported)
            {
                CurrentGpu->PresentFamilyIndex = j;
            }

            if (CurrentGpu->GraphicsFamilyIndex < UINT32_MAX && CurrentGpu->PresentFamilyIndex < UINT32_MAX)
            {
                break;
            }
        }

        if (CurrentGpu->GraphicsFamilyIndex == UINT32_MAX || CurrentGpu->PresentFamilyIndex == UINT32_MAX)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get all required queue indices for device %u "
                     "(graphics %u, present %u)",
                     i + 1, CurrentGpu->GraphicsFamilyIndex, CurrentGpu->PresentFamilyIndex);
            continue;
        }

        LogTrace("Getting extensions");
        UINT32 ExtensionCount = 0;
        Result = vkEnumerateDeviceExtensionProperties(CurrentGpu->Device, NULL, &ExtensionCount,
                                                      NULL);
        if (Result != VK_SUCCESS)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get extensions for device %u: VkResult %d", i + 1, Result);
            continue;
        }
        if (ExtensionCount < 1)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no extensions when "
                     "there are %zu required",
                     i + 1, PURPL_ARRAYSIZE(RequiredDeviceExtensions));
            continue;
        }

        stbds_arrsetlen(CurrentGpu->ExtensionProperties, ExtensionCount);
        Result = vkEnumerateDeviceExtensionProperties(CurrentGpu->Device, NULL, &ExtensionCount,
                                                      CurrentGpu->ExtensionProperties);
        if (Result != VK_SUCCESS)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get extensions for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting surface capabilities");
        Result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(CurrentGpu->Device, VlkData.Surface,
                                                           &CurrentGpu->SurfaceCapabilities);
        if (Result != VK_SUCCESS)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get surface capabilities for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting surface formats");
        UINT32 FormatCount = 0;
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(CurrentGpu->Device, VlkData.Surface, &FormatCount, NULL);
        if (Result != VK_SUCCESS)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get surface formats for device %u: VkResult %d", i + 1, Result);
            continue;
        }
        if (FormatCount < 1)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no surface formats", i + 1);
            continue;
        }

        stbds_arrsetlen(CurrentGpu->SurfaceFormats, FormatCount);
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(CurrentGpu->Device, VlkData.Surface, &FormatCount,
                                                      CurrentGpu->SurfaceFormats);
        if (Result != VK_SUCCESS)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get surface formats for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting present modes");
        UINT32 ModeCount = 0;
        Result = vkGetPhysicalDeviceSurfacePresentModesKHR(CurrentGpu->Device, VlkData.Surface, &ModeCount, NULL);
        if (Result != VK_SUCCESS)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get present modes for device %u: VkResult %d", i + 1, Result);
            continue;
        }
        if (FormatCount < 1)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no present modes", i + 1);
            continue;
        }

        stbds_arrsetlen(CurrentGpu->PresentModes, ModeCount);
        Result = vkGetPhysicalDeviceSurfacePresentModesKHR(CurrentGpu->Device, VlkData.Surface, &ModeCount,
                                                           CurrentGpu->PresentModes);
        if (Result != VK_SUCCESS)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get present modes for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting features");
        vkGetPhysicalDeviceFeatures(CurrentGpu->Device, &CurrentGpu->SupportedFeatures);
        if (!CurrentGpu->SupportedFeatures.samplerAnisotropy)
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it doesn't support sampler "
                     "anisotropy",
                     i + 1);
            continue;
        }

        LogTrace("Getting memory properties");
        vkGetPhysicalDeviceMemoryProperties(CurrentGpu->Device, &CurrentGpu->MemoryProperties);
        for (i = 0; i < CurrentGpu->MemoryProperties.memoryHeapCount; i++)
        {
            LogDebug("Memory heap %d: %s%s", i, CmnFormatSize(CurrentGpu->MemoryProperties.memoryHeaps[i].size),
                     i == 0 ? " (this tends to be the amount of VRAM the GPU has)" : "");
        }

        LogTrace("Getting properties");
        vkGetPhysicalDeviceProperties(CurrentGpu->Device, &CurrentGpu->Properties);

        UsableCount++;
        if (CurrentGpu->Usable)
        {
            VlkData.Gpu = CurrentGpu;
            VlkData.GpuIndex = i;
            LogDebug("Selected device %zu %s [%04x:%04x]", VlkData.GpuIndex, VlkData.Gpu->Properties.deviceName,
                     VlkData.Gpu->Properties.vendorID, VlkData.Gpu->Properties.deviceID);
            break;
        }
    }

    stbds_arrfree(Devices);

    LogDebug("Got information for %d device(s) (of which %d are usable)", stbds_arrlenu(VlkData.Gpus), UsableCount);
    if (UsableCount < 1)
    {
        CmnError("Could not find any usable Vulkan devices");
    }
}

VOID VlkCreateLogicalDevice(VOID)
{
    VkDeviceCreateInfo DeviceCreateInformation = {0};
    VkDeviceQueueCreateInfo QueueCreateInfos[2] = {0};
    FLOAT QueuePriority = 1.0f;

    LogDebug("Creating logical device");

    QueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueCreateInfos[0].queueFamilyIndex = VlkData.Gpu->GraphicsFamilyIndex;
    QueueCreateInfos[0].pQueuePriorities = &QueuePriority;
    QueueCreateInfos[0].queueCount = 1;
    DeviceCreateInformation.queueCreateInfoCount = 1;
    if (VlkData.Gpu->GraphicsFamilyIndex != VlkData.Gpu->PresentFamilyIndex)
    {
        QueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfos[1].queueFamilyIndex = VlkData.Gpu->GraphicsFamilyIndex;
        QueueCreateInfos[1].pQueuePriorities = &QueuePriority;
        QueueCreateInfos[1].queueCount = 1;
        DeviceCreateInformation.queueCreateInfoCount = 2;
    }

    VkPhysicalDeviceFeatures DeviceFeatures = {0};
    DeviceFeatures.samplerAnisotropy = TRUE;

    // VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexingFeatures =
    // {0}; DescriptorIndexingFeatures.sType =
    // VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    // DescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind
    // = TRUE;
    // DescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind =
    // TRUE;

    // VkPhysicalDeviceRobustness2FeaturesEXT DeviceRobustness2Features = {0};
    // DeviceRobustness2Features.sType =
    // VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
    // DeviceRobustness2Features.nullDescriptor = TRUE;
    // DeviceRobustness2Features.pNext = &DescriptorIndexingFeatures;
    
    VkPhysicalDeviceVulkan12Features Device12Features = {0};
    Device12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    Device12Features.bufferDeviceAddress = TRUE;
    Device12Features.bufferDeviceAddressCaptureReplay = TRUE;

    DeviceCreateInformation.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceCreateInformation.pQueueCreateInfos = QueueCreateInfos;
    DeviceCreateInformation.pEnabledFeatures = &DeviceFeatures;
    DeviceCreateInformation.ppEnabledExtensionNames = RequiredDeviceExtensions;
    DeviceCreateInformation.enabledExtensionCount = PURPL_ARRAYSIZE(RequiredDeviceExtensions);
//    DeviceCreateInformation.pNext = &Device12Features;

    LogTrace("Calling vkCreateDevice");
    VULKAN_CHECK(
        vkCreateDevice(VlkData.Gpu->Device, &DeviceCreateInformation, VlkGetAllocationCallbacks(), &VlkData.Device));
    VlkSetObjectName((UINT64)VlkData.Device, VK_OBJECT_TYPE_DEVICE, "Logical device");

    LogDebug("Retrieving queues");

    vkGetDeviceQueue(VlkData.Device, VlkData.Gpu->GraphicsFamilyIndex, 0, &VlkData.GraphicsQueue);
    VlkSetObjectName((UINT64)VlkData.GraphicsQueue, VK_OBJECT_TYPE_QUEUE, "Graphics queue");
    vkGetDeviceQueue(VlkData.Device, VlkData.Gpu->PresentFamilyIndex, 0, &VlkData.PresentQueue);
    VlkSetObjectName((UINT64)VlkData.PresentQueue, VK_OBJECT_TYPE_QUEUE, "Presentation queue");

    VlkSetObjectName((UINT64)VlkData.Instance, VK_OBJECT_TYPE_INSTANCE, "Instance");

    VlkSetObjectName((UINT64)VlkData.Surface, VK_OBJECT_TYPE_SURFACE_KHR, "Surface");

    LogDebug("Loading device functions");
    volkLoadDevice(VlkData.Device);
}
