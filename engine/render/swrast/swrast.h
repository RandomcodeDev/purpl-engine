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

extern VOID SwrsPutPixel(_In_ CONST ivec2 Position, _In_ CONST vec4 Colour);

extern VOID SwrsProjectVertex(_Inout_ PMESH_VERTEX Vertex, _In_ mat4 Transform);

extern VOID SwrsDrawLine(_In_ CONST MESH_VERTEX Start, _In_ CONST MESH_VERTEX End, _In_opt_ CONST mat4 Transform,
                         _In_ BOOLEAN Project);

extern VOID SwrsDrawTriangle(_In_ CONST MESH_VERTEX First, _In_ CONST MESH_VERTEX Second, _In_ CONST MESH_VERTEX Third,
                             _In_ BOOLEAN Filled);

extern VOID SwrsDrawModel(_In_ PMODEL Model, _In_ PRENDER_OBJECT_UNIFORM Uniform, _In_ PRENDER_OBJECT_DATA Data);
