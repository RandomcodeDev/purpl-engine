/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    vk.h

Abstract:

    This module contains definitions used by the Vulkan backend.

--*/

#pragma once

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#include "common/common.h"
#include "common/log.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"

#include "render.h"
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
    CmnError("VMA assertion failed: " #expr); \
}
#ifdef PURPL_VULKAN_DEBUG
#define VMA_STATS_STRING_ENABLED 1
#endif
#include "vk_mem_alloc.h"

#define VULKAN_FRAME_COUNT 3
