/*++

Copyright (c) 2023 Randomcode Developers

Module Name:

    texture.h

Abstract:

    This file implements the texture format routines.

--*/

#include "texture.h"

static UINT8 FormatComponents[TextureFormatCount] = {
    0, // TextureFormatUndefined
    1, // TextureFormatDepth
    4, // TextureFormatRgba8
    3, // TextureFormatRgb8
};

static UINT8 FormatPitches[TextureFormatCount] = {
    0, // TextureFormatUndefined
    4, // TextureFormatDepth
    4, // TextureFormatRgba8
    3, // TextureFormatRgb8
};

PTEXTURE
CreateTexture(_In_ TEXTURE_FORMAT Format, _In_ UINT32 Width, _In_ UINT32 Height, _In_ PVOID Data)
/*++

Routine Description:

    Creates a texture.

Arguments:

    Format - The format of the texture.

    Width - The width of the texture in pixels.

    Height - The height of the texture in pixels.

    Data - The pixels in the texture. NULL for all black.

Return Value:

    The newly created texture, or NULL in the event of failure.

--*/
{
    PTEXTURE Texture;
    SIZE_T Size;

    if (Format <= TextureFormatUndefined || Format >= TextureFormatCount || Width <= 0 || Height <= 0)
    {
        LogError("Cannot create invalid texture");
        return NULL;
    }

    Size = sizeof(TEXTURE);
    if (!Data)
    {
        Size += EstimateTextureSize(Format, Width, Height);
    }

    Texture = CmnAlloc(1, Size);
    if (!Texture)
    {
        LogError("Could not allocate texture");
        return NULL;
    }

    Texture->Magic = TEXTURE_MAGIC_NUMBER;
    Texture->Version = TEXTURE_FORMAT_VERSION;
    Texture->Format = Format;
    Texture->Width = Width;
    Texture->Height = Height;
    if (Data)
    {
        Texture->Pixels = Data;
        Texture->DataSeparate = TRUE;
    }
    else
    {
        Texture->Pixels = Texture + sizeof(TEXTURE);
    }

    return Texture;
}

PTEXTURE
LoadTexture(_In_ PCSTR Path)
/*++

Routine Description:

    Loads a texture from the given path and parses it.

Arguments:

    Path - The path of the texture to load.

Return Value:

    The loaded texture or NULL if something went wrong.

--*/
{
    PTEXTURE Texture;
    PTEXTURE RealTexture;
    SIZE_T RequiredSize;
    SIZE_T Size;
    PVOID Data;

    LogInfo("Loading texture %s", Path);

    Size = 0;
    Texture = FsReadFile(Path, 0, 0, &Size, sizeof(TEXTURE) - TEXTURE_HEADER_SIZE);
    if (!ValidateTexture(Texture))
    {
        LogError("Texture %s is invalid", Path);
        return NULL;
    }

    LogDebug("Validating texture");
    RequiredSize = TEXTURE_HEADER_SIZE;
    if (Size < RequiredSize)
    {
        LogError("Texture is %zu bytes but should be %zu bytes", Size, RequiredSize);
        CmnFree(Texture);
        return NULL;
    }

    Data = (PBYTE)Texture + TEXTURE_HEADER_SIZE;

    RealTexture = CmnAlloc(1, sizeof(TEXTURE) + GetTextureSize(*Texture));
    if (!RealTexture)
    {
        LogError("Failed to allocate %zu bytes for texture", sizeof(TEXTURE) + GetTextureSize(*Texture));
        CmnFree(Texture);
        return NULL;
    }

    LogDebug("Decompressing texture");
    memcpy(RealTexture, Texture, sizeof(TEXTURE));
    RealTexture->DataSeparate = FALSE;
    RealTexture->Pixels = (PBYTE)RealTexture + sizeof(TEXTURE);
    if (ZSTD_decompress(RealTexture->Pixels, GetTextureSize(*Texture), Data, Texture->CompressedSize) !=
        GetTextureSize(*Texture))
    {
        LogError("Decompressed pixels are not the expected size");
        CmnFree(RealTexture);
        CmnFree(Texture);
        return NULL;
    }

    CmnFree(Texture);

    return RealTexture;
}

BOOLEAN
WriteTexture(_In_ PCSTR Path, _In_ PTEXTURE Texture)
/*++

Routine Description:

    Writes the given texture to the path specified.

Arguments:

    Path - The path of the file to write the texture to.

    Texture - The texture to write.

Return Value:

    TRUE - Success.

    FALSE - Failure.

--*/
{
    PVOID Data;

    if (!Path || !Texture)
    {
        return FALSE;
    }

    LogInfo("Writing texture to %s", Path);

    Data = CmnAlloc(1, ZSTD_COMPRESSBOUND(GetTextureSize(*Texture)));
    if (!Data)
    {
        LogError("Failed to allocate memory for texture compression");
        return FALSE;
    }

    LogDebug("Compressing texture");
    Texture->CompressedSize = ZSTD_compress(Data, ZSTD_COMPRESSBOUND(GetTextureSize(*Texture)), Texture->Pixels,
                                            GetTextureSize(*Texture), ZSTD_btultra2);
    if (ZSTD_isError(Texture->CompressedSize))
    {
        LogError("Failed to compress texture");
        return FALSE;
    }

    LogDebug("Writing texture header");
    if (!FsWriteFile(Path, Texture, TEXTURE_HEADER_SIZE, FALSE))
    {
        LogError("Could not write texture header to %s", Path);
        return FALSE;
    }
    LogDebug("Writing texture data");
    if (!FsWriteFile(Path, Data, Texture->CompressedSize, TRUE))
    {
        LogError("Could not write texture data to %s", Path);
        return FALSE;
    }

    CmnFree(Data);

    return TRUE;
}

SIZE_T
GetFormatComponents(_In_ TEXTURE_FORMAT Format)
/*++

Routine Description:

    Gets the number of compenents of the given format.

Arguments:

    Format - The texture format.

Return Value:

    The number of components of the format or 0 if it is invalid.

--*/
{
    return FormatComponents[PURPL_CLAMP(Format, TextureFormatUndefined, TextureFormatCount - 1)];
}

SIZE_T
GetFormatPitch(_In_ TEXTURE_FORMAT Format)
/*++

Routine Description:

    Gets the pitch (bytes per pixel) of the given format.

Arguments:

    Format - The texture format.

Return Value:

    The pitch of the format or 0 if it is invalid.

--*/
{
    return FormatPitches[PURPL_CLAMP(Format, TextureFormatUndefined, TextureFormatCount - 1)];
}

SIZE_T
EstimateTextureSize(_In_ TEXTURE_FORMAT Format, _In_ UINT32 Width, _In_ UINT32 Height)
/*++

Routine Description:

    Calculates the expected size of a texture.

Arguments:

    Format - The format of the texture.

    Width - The width of the texture in pixels.

    Height - The height of the texture in pixels.

Return Value:

    The size the described texture should be.

--*/
{
    return Width * Height * GetFormatPitch(Format);
}
