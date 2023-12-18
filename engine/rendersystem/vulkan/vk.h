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
CONST VkAllocationCallbacks*
VulkanGetAllocationCallbacks(
    VOID
    );

//
// Number of frames in flight at a time
//

#define VULKAN_FRAME_COUNT 3

//
// Struct declarations
//

typedef struct RENDERABLE RENDERABLE, *PRENDERABLE;
typedef struct RENDER_GLOBAL_UNIFORM_DATA RENDER_GLOBAL_UNIFORM_DATA, *PRENDER_GLOBAL_UNIFORM_DATA;
typedef struct RENDER_MODEL_UNIFORM_DATA RENDER_MODEL_UNIFORM_DATA, *PRENDER_MODEL_UNIFORM_DATA;
typedef struct SHADER SHADER, *PSHADER;
typedef struct RENDER_TEXTURE RENDER_TEXTURE, *PRENDER_TEXTURE;
typedef struct RENDER_FONT RENDER_FONT, *PRENDER_FONT;
typedef struct GLYPH GLYPH, *PGLYPH;
typedef struct MODEL MODEL, *PMODEL;

//
// Initialize Vulkan
//

extern
VOID
VulkanInitialize(
    VOID
    );

//
// Set up command recording
//

extern
VOID
VulkanBeginCommands(
    _In_ PRENDER_GLOBAL_UNIFORM_DATA UniformData,
    _In_ BOOLEAN WindowResized
    );

//
// Draw a model
//

extern
VOID
VulkanDrawModel(
    _In_ PMODEL Model,
    _In_ PRENDER_MODEL_UNIFORM_DATA UniformData
    );

//
// Present the swap chain
//

extern
VOID
VulkanPresentFrame(
    VOID
    );

//
// Shut down Vulkan
//

extern
VOID
VulkanShutdown(
    VOID
    );

//
// Create a shader
//

extern
VOID
VulkanCreateShader(
    _In_ PSHADER SourceShader
    );

//
// Destroy a shader
//

extern
VOID
VulkanDestroyShader(
    _In_ PSHADER Shader
    );

//
// Use a model
//

extern
VOID
VulkanUseMesh(
    _In_ PMODEL SourceModel
    );

//
// Destroy a model
//

extern
VOID
VulkanDestroyModel(
    _In_ PMODEL Model
    );

//
// Use a texture
//

extern
VOID
VulkanUseTexture(
    _In_ PRENDER_TEXTURE Texture
    );

//
// Destroy a texture
//

extern
VOID
VulkanDestroyTexture(
    _In_ PRENDER_TEXTURE Texture
    );

//
// Use a font
//

extern
VOID
VulkanUseFont(
    _In_ PRENDER_FONT SourceFont
    );

//
// Use a font
//

extern
VOID
VulkanDestroyFont(
    _In_ PRENDER_FONT Font
    );

//
// Draw a glyph
//

extern
VOID
VulkanDrawGlyph(
    _In_ PRENDER_FONT Font,
    _In_ FLOAT Scale,
    _In_ vec4 Colour,
    _In_ vec2 Position,
    _In_ PGLYPH Glyph,
    _In_ SIZE_T Offset
    );
