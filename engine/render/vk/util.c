/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    util.c

Abstract:

    This file implements miscellaneous Vulkan helper functions, including
    moving images and buffers around, and getting human readable information.

--*/

#include "vk.h"

static PVOID VKAPI_PTR Allocate(PVOID pUserData, SIZE_T size, SIZE_T alignment, VkSystemAllocationScope allocationScope)
{
    UNREFERENCED_PARAMETER(pUserData);
    UNREFERENCED_PARAMETER(allocationScope);
    return CmnAlignedAlloc(alignment, size);
}

static void VKAPI_PTR Free(PVOID pUserData, PVOID pMemory)
{
    UNREFERENCED_PARAMETER(pUserData);
    CmnAlignedFree(pMemory);
}

static void VKAPI_PTR LogInternalAllocation(PVOID pUserData, SIZE_T size, VkInternalAllocationType allocationType,
                                  VkSystemAllocationScope allocationScope)
{
    UNREFERENCED_PARAMETER(pUserData);
    LogTrace("Vulkan allocation made:");
    LogTrace("\tSize: %zu", size);
    LogTrace("\tType: %d", allocationType);
    LogTrace("\tScope: %d", allocationScope);
}

static void VKAPI_PTR LogInternalFree(PVOID pUserData, SIZE_T size, VkInternalAllocationType allocationType,
                            VkSystemAllocationScope allocationScope)
{
    UNREFERENCED_PARAMETER(pUserData);
    LogTrace("Vulkan allocation freed:");
    LogTrace("\tSize: %zu", size);
    LogTrace("\tType: %d", allocationType);
    LogTrace("\tScope: %d", allocationScope);
}

static PVOID VKAPI_PTR Reallocate(PVOID pUserData, PVOID pOriginal, SIZE_T size, SIZE_T alignment,
                        VkSystemAllocationScope allocationScope)
{
    UNREFERENCED_PARAMETER(pUserData);
    UNREFERENCED_PARAMETER(allocationScope);
    return CmnAlignedRealloc(pOriginal, alignment, size);
}

static CONST VkAllocationCallbacks AllocationCallbacks = {
    .pfnAllocation = Allocate,
    .pfnReallocation = Reallocate,
    .pfnFree = Free,
    .pfnInternalAllocation = LogInternalAllocation,
    .pfnInternalFree = LogInternalFree,
};

CONST VkAllocationCallbacks *VlkGetAllocationCallbacks(VOID)
{
    return &AllocationCallbacks;
}

PCSTR VlkGetResultString(VkResult Result)
{
#define X(Result) \
    case Result: \
        return PURPL_STRINGIZE(Result);

    switch (Result)
    {
    X(VK_SUCCESS)
    X(VK_NOT_READY)
    X(VK_TIMEOUT)
    X(VK_EVENT_SET)
    X(VK_EVENT_RESET)
    X(VK_INCOMPLETE)
    X(VK_ERROR_OUT_OF_HOST_MEMORY)
    X(VK_ERROR_OUT_OF_DEVICE_MEMORY)
    X(VK_ERROR_INITIALIZATION_FAILED)
    X(VK_ERROR_DEVICE_LOST)
    X(VK_ERROR_MEMORY_MAP_FAILED)
    X(VK_ERROR_LAYER_NOT_PRESENT)
    X(VK_ERROR_EXTENSION_NOT_PRESENT)
    X(VK_ERROR_FEATURE_NOT_PRESENT)
    X(VK_ERROR_INCOMPATIBLE_DRIVER)
    X(VK_ERROR_TOO_MANY_OBJECTS)
    X(VK_ERROR_FORMAT_NOT_SUPPORTED)
    X(VK_ERROR_FRAGMENTED_POOL)
    X(VK_ERROR_UNKNOWN)
    X(VK_ERROR_OUT_OF_POOL_MEMORY)
    X(VK_ERROR_INVALID_EXTERNAL_HANDLE)
    X(VK_ERROR_FRAGMENTATION)
    X(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
    X(VK_ERROR_SURFACE_LOST_KHR)
    X(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
    X(VK_SUBOPTIMAL_KHR)
    X(VK_ERROR_OUT_OF_DATE_KHR)
    X(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
    X(VK_ERROR_VALIDATION_FAILED_EXT)
    X(VK_ERROR_INVALID_SHADER_NV)
    X(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
    X(VK_ERROR_NOT_PERMITTED_EXT)
    X(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
    X(VK_THREAD_IDLE_KHR)
    X(VK_THREAD_DONE_KHR)
    X(VK_OPERATION_DEFERRED_KHR)
    X(VK_OPERATION_NOT_DEFERRED_KHR)
    X(VK_PIPELINE_COMPILE_REQUIRED_EXT)
#ifndef PURPL_SWITCH
    X(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
    X(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
    X(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
    X(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
    X(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
    X(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
#endif
    default:
        return "VK_UNKNOWN";
    }

#undef X
}

VOID VlkSetObjectName(_In_ UINT64 Object, _In_ VkObjectType ObjectType, _In_z_ _Printf_format_string_ PCSTR Name, ...)
{
    //  Seems to crash a lot
#ifndef PURPL_SWITCH
#ifdef PURPL_VULKAN_DEBUG
    if (ObjectType == VK_OBJECT_TYPE_BUFFER)
    {
        LogWarning("You may want to use VlkNameBuffer");
    }

    // TODO: Make constant somewhere or do this in a better way?
    if (ObjectType == 0x69420BFF)
    {
        ObjectType = VK_OBJECT_TYPE_BUFFER;
    }

    if (vkSetDebugUtilsObjectNameEXT && VlkData.Device)
    {
        va_list Arguments;

        VkDebugUtilsObjectNameInfoEXT NameInformation = {0};
        NameInformation.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        NameInformation.objectType = ObjectType;
        NameInformation.objectHandle = (UINT64)Object;

        va_start(Arguments, Name);
        NameInformation.pObjectName = CmnFormatStringVarArgs(Name, Arguments);
        va_end(Arguments);

        LogTrace("Setting object name of type %u object 0x%llX to %s", ObjectType, (UINT64)Object,
                 NameInformation.pObjectName);

        vkSetDebugUtilsObjectNameEXT(VlkData.Device, &NameInformation);

        CmnFree(NameInformation.pObjectName);
    }

    return;
#endif
#endif

    UNREFERENCED_PARAMETER(Object);
    UNREFERENCED_PARAMETER(ObjectType);
    UNREFERENCED_PARAMETER(Name);
}

VkBool32 VKAPI_PTR VlkDebugCallback(_In_ VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
                                     _In_ VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
                                     _In_ CONST VkDebugUtilsMessengerCallbackDataEXT *CallbackData,
                                     _In_opt_ PVOID UserData)
{
    CHAR Type[128];
    LOG_LEVEL Level;

    UNREFERENCED_PARAMETER(UserData);

    switch (MessageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        Level = LogLevelDebug;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        Level = LogLevelInfo;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        Level = LogLevelWarning;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        Level = LogLevelError;
        break;
    default:
        Level = LogLevelInfo;
        break;
    }

    snprintf(Type, PURPL_ARRAYSIZE(Type), "%s%s%smessage",
             (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) ? "general " : "",
             (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) ? "performance " : "",
             (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ? "validation " : "");
    Type[0] = (CHAR)toupper(Type[0]);
    LogMessage(Level,
               "Vulkan",                       // "File"
               (UINT64)PlatGetReturnAddress(), // "Line"
               true,                        // Display it in hex cause it's an address
               "%s: %s", Type, CallbackData->pMessage);

    return TRUE;
}

VkCommandBuffer VlkBeginTransfer(VOID)
{
    VkCommandBufferAllocateInfo CommandBufferAllocateInformation = {0};

    CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInformation.commandPool = VlkData.TransferCommandPool;
    CommandBufferAllocateInformation.commandBufferCount = 1;

    // LogTrace("Creating transfer command buffer");
    VkCommandBuffer TransferBuffer;
    VULKAN_CHECK(vkAllocateCommandBuffers(VlkData.Device, &CommandBufferAllocateInformation, &TransferBuffer));

    VkCommandBufferBeginInfo BeginInformation = {0};
    BeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // LogTrace("Beginning transfer command buffer");
    vkBeginCommandBuffer(TransferBuffer, &BeginInformation);

    return TransferBuffer;
}

VOID VlkEndTransfer(_In_ VkCommandBuffer TransferBuffer)
{
    // LogTrace("Ending transfer command buffer");
    vkEndCommandBuffer(TransferBuffer);

    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInformation.commandBufferCount = 1;
    SubmitInformation.pCommandBuffers = &TransferBuffer;

    // LogTrace("Submitting transfer command buffer");
    vkQueueSubmit(VlkData.GraphicsQueue, 1, &SubmitInformation, VK_NULL_HANDLE);
    vkQueueWaitIdle(VlkData.GraphicsQueue);

    // LogTrace("Destroying transfer command buffer");
    vkFreeCommandBuffers(VlkData.Device, VlkData.TransferCommandPool, 1, &TransferBuffer);
}
