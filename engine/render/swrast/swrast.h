/// @file swrast.h
///
/// @brief This file contains definitions used by the software rasterizer, which is based on
/// https://github.com/ssloy/tinyrenderer/wiki
///
/// @copyright (c) Randomcode Developers

#pragma once

#include "common/alloc.h"
#include "common/common.h"
#include "common/log.h"

#include "engine/mathutil.h"

#include "engine/render/render.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"

/// @brief Make a pixel
#define SWRS_PIXEL(R, G, B, A) VidConvertPixel((R) << 24 | (G) << 16 | (B) << 8 | (A))

/// @brief Data used by the backend
typedef struct SOFTWARE_RASTERIZER_DATA
{
    PVIDEO_FRAMEBUFFER Framebuffer;
} SOFTWARE_RASTERIZER_DATA, *PSOFTWARE_RASTERIZER_DATA;
extern SOFTWARE_RASTERIZER_DATA SwrsData;

/// @brief Set a pixel
///
/// @param[in] X The X coordinate of the pixel
/// @param[in] Y The Y coordinate of the pixel
/// @param[in] Pixel The pixel to set
extern VOID SwrsSetPixel(_In_ UINT32 X, _In_ UINT32 Y, _In_ UINT32 Pixel);

/// @brief Draw a line
///
/// @param[in] Start The starting point
/// @param[in] End The ending point
/// @param[in] Pixel The pixel to write (in framebuffer format)
extern VOID SwrsDrawLine(_In_ ivec2 Start, _In_ ivec2 End, _In_ UINT32 Pixel);

/// @brief Draw a model
///
/// @param[in] Model The model to draw
/// @param[in] Transform Where to draw the model
extern VOID SwrsDrawModel(_In_ PMODEL Model, _In_ mat4 Object, _In_ mat4 World, _In_ mat4 Projection);
