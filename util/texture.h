/// @file texture.h
///
/// @brief This file defines the texture API
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/filesystem.h"
#include "common/log.h"

/// @brief Texture file magic string
#define TEXTURE_MAGIC "PTEX"
/// @brief Texture file magic number (little endian)
#define TEXTURE_MAGIC_NUMBER 'XETP'
/// @brief Texture file magic signature length
#define TEXTURE_MAGIC_LENGTH 4

/// @brief The revision of the texture format
#define TEXTURE_FORMAT_VERSION 2

/// @brief Texture formats. Only TextureFormatRgba8 is used.
typedef enum TEXTURE_FORMAT
{
    TextureFormatUndefined,
    TextureFormatDepth,
    TextureFormatRgba8,
    TextureFormatRgb8,
    TextureFormatCount
} TEXTURE_FORMAT, *PTEXTURE_FORMAT;

/// @brief A depth pixel
typedef union DEPTH_PIXEL {
    BYTE Bytes[4];
    FLOAT Value;
} DEPTH_PIXEL, *PDEPTH_PIXEL;

/// @brief An RGBA pixel
typedef union RGBA8_PIXEL {
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

/// @brief An RGB pixel
typedef union RGB8_PIXEL {
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

/// @brief A texture
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

    // Following fields are in memory only

    BOOLEAN DataSeparate;
    union {
        PDEPTH_PIXEL DepthPixels;
        PRGBA8_PIXEL Rgba8Pixels;
        PRGB8_PIXEL Rgb8Pixels;
        PVOID Pixels;
        UINT64 AlignmentDummy; // Ensures alignment remains the same
    };
} TEXTURE, *PTEXTURE;

/// @brief Check a texture's validity
///
/// @param Texture The texture to check
///
/// @return Whether the texture can be used
#define ValidateTexture(Texture)                                               \
    ((Texture) &&                                                              \
     memcmp((Texture)->MagicBytes, TEXTURE_MAGIC, TEXTURE_MAGIC_LENGTH) ==     \
         0 &&                                                                  \
     (Texture)->Version == TEXTURE_FORMAT_VERSION &&                           \
     (Texture)->Format > TextureFormatUndefined &&                             \
     (Texture)->Format < TextureFormatCount && (Texture)->Width >= 1 &&        \
     (Texture)->Height >= 1)

/// @brief Create a texture
///
/// @param Format The format of the texture
/// @param Width The width of the texture
/// @param Height The height of the texture
/// @param Data The pixel data for the image (should be at least
/// EstimateTextureSize bytes)
///
/// @return A texture which can be freed with CmnFree
extern PTEXTURE CreateTexture(_In_ TEXTURE_FORMAT Format, _In_ UINT32 Width,
                              _In_ UINT32 Height, _In_ PVOID Data);

/// @brief Load a texture from a file
///
/// @param Path The path to the texture file
///
/// @return The loaded texture, which can be freed with CmnFree
extern PTEXTURE LoadTexture(_In_ PCSTR Path);

/// @brief Write a texture to a file
///
/// @param Path The path to write the texture to
/// @param Texture The texture to write
///
/// @return Whether the texture could be written
extern BOOLEAN WriteTexture(_In_ PCSTR Path, _In_ PTEXTURE Texture);

/// @brief Get the number of components in a format's pixels
///
/// @param[in] Format The format
///
/// @return The number of channels in the format
extern SIZE_T GetFormatComponents(_In_ TEXTURE_FORMAT Format);

/// @brief Get the number of bytes per pixel in a format
///
/// @param[in] Format The format to get the pitch of
///
/// @return The number of bytes per pixel
extern SIZE_T GetFormatPitch(_In_ TEXTURE_FORMAT Format);

/// @brief Size of the texture header
#define TEXTURE_HEADER_SIZE offsetof(TEXTURE, DataSeparate)

/// @brief Get the expected size of a texture's pixel data
///
/// @param[in] Texture The texture to get the size of
///
/// @return The size of the pixel data
#define GetTextureSize(Texture)                                                \
    ((Texture).Width * (Texture).Height * GetFormatPitch((Texture).Format))

/// @brief Get the size of a texture
///
/// @param[in] Format The format
/// @param[in] Width The width
/// @param[in] Height The height
///
/// @return The size the texture's pixel data should be
extern SIZE_T EstimateTextureSize(_In_ TEXTURE_FORMAT Format, _In_ UINT32 Width,
                                  _In_ UINT32 Height);

/// @brief Get the address of a pixel by its position
///
/// @param[in] Texture The texture to get the pixel from
/// @param[in] X The X position of the pixel
/// @param[in] Y The Y position of the pixel
///
/// @return The address of the pixel
#define GetTexturePixel(Texture, X, Y)                                         \
    ((PBYTE)(Texture).Pixels + PURPL_CLAMP(Y, 0, (Texture).Height)             \
         ? (GetFormatPitch((Texture).Format) *                                 \
            PURPL_CLAMP(Y, 0, (Texture).Height) * (Texture).Width)             \
     : 0 + PURPL_CLAMP(X, 0, (Texture).Width)                                  \
         ? (GetFormatPitch((Texture).Format) *                                 \
            PURPL_CLAMP(X, 0, (Texture).Width))                                \
         : 0)
