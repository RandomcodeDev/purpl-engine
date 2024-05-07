/// @file swrast.h
///
/// @brief This file contains the definitions used by the software rasterizer backend.
///
/// This backend is based on the book Computer Graphics from Scratch by Gabriel Gambetta, among other sources.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "common/alloc.h"
#include "common/common.h"
#include "common/log.h"

#include "engine/engine.h"

#include "engine/render/render.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"

PURPL_MAKE_TAG(struct, SWRAST_DATA, {
    PVIDEO_FRAMEBUFFER Framebuffer;
    mat4 ViewProjection;
})
extern SWRAST_DATA SwrsData;

extern VOID SwrsPutPixel(_In_ vec2 Position, _In_ vec4 Colour);

extern VOID SwrsDrawLine(_In_ vec3 Start, _In_ vec3 End, _In_ vec4 Colour, _In_ BOOLEAN Project);
