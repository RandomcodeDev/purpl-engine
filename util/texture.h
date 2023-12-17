/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    texture.h

Abstract:

    This module defines the texture API. It could be made more portable, but little endian is more or less everywhere now.

--*/

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/filesystem.h"
#include "common/log.h"

//
// Texture file magic bytes
//

#define TEXTURE_MAGIC "PTEX"
#define TEXTURE_MAGIC_NUMBER 'XETP'
#define TEXTURE_MAGIC_LENGTH 4

//
// Texture format version
//

#define TEXTURE_FORMAT_VERSION 2

//
// Texture formats
//

typedef enum TEXTURE_FORMAT
{
    TextureFormatUndefined,
    TextureFormatDepth,
    TextureFormatRgba8,
    TextureFormatRgb8,
    TextureFormatCount
} TEXTURE_FORMAT, *PTEXTURE_FORMAT;

//
// Pixel types
//

typedef union DEPTH_PIXEL
{
    BYTE Bytes[4];
    FLOAT Value;
} DEPTH_PIXEL, *PDEPTH_PIXEL;
typedef union RGBA8_PIXEL
{
    BYTE Bytes[4];
    UINT32 Value;
    struct RGBA8_PIXEL_FIELDS
    {
        BYTE Blue;
        BYTE Green;
        BYTE Red;
        BYTE Alpha;
    };
} RGBA8_PIXEL, *PRGBA8_PIXEL;
typedef union RGB8_PIXEL
{
    BYTE Bytes[3];
#if __STDC_VERSION__ >= 202300L
    _BitInt(24) Value;
#else
    UINT32 Value;
#endif
    struct RGB8_PIXEL_FIELDS
    {
        BYTE Blue;
        BYTE Green;
        BYTE Red;
    };
} RGB8_PIXEL, *PRGB8_PIXEL;

//
// Texture
//

typedef struct TEXTURE
{
    union {
        CHAR MagicBytes[4];
        UINT32 Magic;
    };
    UINT32 Version;
    TEXTURE_FORMAT Format;
    SIZE_T CompressedSize;
    UINT32 Width;
    UINT32 Height;


    // In memory only
    BOOLEAN DataSeparate;
    union
    {
        PDEPTH_PIXEL DepthPixels;
        PRGBA8_PIXEL Rgba8Pixels;
        PRGB8_PIXEL Rgb8Pixels;
        PVOID Pixels;
        UINT64 AlignmentDummy; // Ensures alignment remains
    };
} TEXTURE, *PTEXTURE;

//
// Check a texture's validity
//

#define ValidateTexture(Texture) \
    ((Texture) && \
    memcmp( \
        (Texture)->MagicBytes, \
        TEXTURE_MAGIC, \
        TEXTURE_MAGIC_LENGTH \
        ) == 0 && \
    (Texture)->Version == TEXTURE_FORMAT_VERSION && \
    (Texture)->Format > TextureFormatUndefined && \
    (Texture)->Format < TextureFormatCount && \
    (Texture)->Width >= 1 && (Texture)->Height >= 1)

//
// Create a texture
//

extern
PTEXTURE
CreateTexture(
    _In_ TEXTURE_FORMAT Format,
    _In_ UINT32 Width,
    _In_ UINT32 Height,
    _In_ PVOID Data
    );

//
// Load a texture from a file
//

extern
PTEXTURE
LoadTexture(
    _In_ PCSTR Path
    );

//
// Write a texture to a file
//

extern
BOOLEAN
WriteTexture(
    _In_ PCSTR Path,
    _In_ PTEXTURE Texture
    );

//
// Get the number of components of a texture format
//

extern
SIZE_T
GetFormatComponents(
    _In_ TEXTURE_FORMAT Format
    );

//
// Get the pitch of a texture format
//

extern
SIZE_T
GetFormatPitch(
    _In_ TEXTURE_FORMAT Format
    );

//
// Texture header size
//

#define TEXTURE_HEADER_SIZE offsetof(TEXTURE, DataSeparate)

//
// Get the size of a texture
//

#define GetTextureSize(Texture) \
    ((Texture).Width * (Texture).Height * GetFormatPitch((Texture).Format))

//
// Calculates the expected size of a texture
//

extern
SIZE_T
EstimateTextureSize(
    _In_ TEXTURE_FORMAT Format,
    _In_ UINT32 Width,
    _In_ UINT32 Height
    );

//
// Gets the address of a given pixel
//

#define GetTexturePixel(Texture, X, Y) \
    ((PVOID)(PBYTE)(Texture).Pixels + (Y) ? \
        (GetFormatPitch((Texture).Format) * (Y) * (Texture).Width) : \
        0 + (X) ? (GetFormatPitch((Texture).Format) * (X)) : 0)
