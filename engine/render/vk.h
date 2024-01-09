/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    vk.h

Abstract:

    This module defines the Vulkan render interface.

--*/

#pragma once

#include "volk.h"

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#include "common/common.h"
#include "common/log.h"
END_EXTERN_C

#if defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO
#define PURPL_VULKAN_DEBUG 1
#endif

#define VMA_VULKAN_VERSION 1003000
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
//#define VMA_SYSTEM_ALIGNED_MALLOC PURPL_ALIGNED_ALLOC
//#define VMA_SYSTEM_ALIGNED_FREE PURPL_ALIGNED_FREE
#define VMA_DEBUG_LOG_FORMAT(format, ...) LogDebug((format), __VA_ARGS__)
#define VMA_HEAVY_ASSERT(expr) if ( !(expr) ) \
{ \
    CommonError("VMA assertion failed: " #expr); \
}
#ifdef PURPL_VULKAN_DEBUG
#define VMA_STATS_STRING_ENABLED 1
#endif
#include "vk_mem_alloc.h"

//
// For platform functions
//

extern
CONST
VkAllocationCallbacks*
VulkanGetAllocationCallbacks(
    VOID
    );

typedef struct RENDER_BACKEND RENDER_BACKEND, *PRENDER_BACKEND;

//
// Sets up the backend
//

extern
VOID
VulkanSetupBackend(
    _In_ PRENDER_BACKEND Backend
    );
