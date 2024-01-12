/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    util.c

Abstract:

    This module implements miscellaneous Vulkan helper functions, including
    moving images and buffers around, and getting human readable information.

--*/

#include "vk.h"

PCSTR
VlkGetResultString(
    VkResult Result
    )
/*++

Routine Description:

    Gets a human-readable version of a VkResult.

Arguments:

    Result - The result to get the string for.

Return Value:

    A string describing the result.

--*/
{
    // Stolen from https://github.com/libsdl-org/SDL/blob/a4c6b38fef2ff046e38072200b096c03389bfa28/src/video/SDL_vulkan_utils.c#L29,
    // but it's just a table that I could probably generate somehow (and, in fact, should)
    switch (Result)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
        return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
        return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
#if VK_HEADER_VERSION >= 135 && VK_HEADER_VERSION < 162
    case VK_ERROR_INCOMPATIBLE_VERSION_KHR:
        return "VK_ERROR_INCOMPATIBLE_VERSION_KHR";
#endif
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT:
        return "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:
        return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
        return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
        return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
        return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
    default:
        return "VK_UNKNOWN";
        break;
    }
}

VOID
VlkSetObjectName(
    _In_ PVOID Object,
    _In_ VkObjectType ObjectType,
    _In_ _Printf_format_string_ PCSTR Name,
    ...
    )
{
    //UNREFERENCED_PARAMETER(Object);
    //UNREFERENCED_PARAMETER(ObjectType);
    //UNREFERENCED_PARAMETER(Name);
    // Seems to crash a lot
#ifdef PURPL_VULKAN_DEBUG
    if ( vkSetDebugUtilsObjectNameEXT && VlkData.Device )
    {
        va_list Arguments;

        LogTrace("Setting object name of type %u object 0x%llX to %s", ObjectType, (UINT64)Object, Name);

        VkDebugUtilsObjectNameInfoEXT NameInformation = {0};
        NameInformation.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        NameInformation.objectType = ObjectType;
        NameInformation.objectHandle = (UINT64)Object;

        va_start(
            Arguments,
            Name
            );
        NameInformation.pObjectName = CmnFormatTempStringVarArgs(
            Name,
            Arguments
            );
        va_end(Arguments);

        vkSetDebugUtilsObjectNameEXT(
            VlkData.Device,
            &NameInformation
            );
    }
#endif
}

VkBool32
VKAPI_CALL
VlkDebugCallback(
    _In_ VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    _In_ VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
    _In_ CONST VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    _In_opt_ PVOID UserData
    )
/*++

Routine Description:

    This routine logs Vulkan messages.

Arguments:

    MessageSeverity - The severity of the message.

    MessageTypes - The types of the message.

    CallbackData - Information about the callback.

    UserData - User data, unused.

Return Value:

    TRUE - Always returned.

--*/
{
    CHAR Location[128];
    LOG_LEVEL Level;

    UNREFERENCED_PARAMETER(UserData);

    switch ( MessageSeverity )
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

    // Put the message types as the source file, using the address of the function as the line
    snprintf(
        Location,
        PURPL_ARRAYSIZE(Location),
        "VULKAN %s%s%sMESSAGE",
        (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) ? "GENERAL " : "",
        (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) ? "PERFORMANCE " : "",
        (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ? "VALIDATION " : ""
        );
    LogMessage(
        Level,
        Location,
        (UINT64)PlatGetReturnAddress(),
        true,
        "%s",
        CallbackData->pMessage
        );

    return TRUE;
}
