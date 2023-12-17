/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    font.c

Abstract:

    This module implements the font API.

--*/

#include "font.h"
#include "cJSON.h"

PFONT
LoadFont(
    _In_ PCSTR AtlasPath,
    _In_ PCSTR AtlasIndexPath
    )
/*++

Routine Description:

    Loads a font and figures out the glyphs.

Arguments:

    AtlasPath - The path to the atlas texture.
    AtlasIndexPath - The path to the JSON index of the atlas.

Return Value:

    NULL on failure, otherwise the loaded font.

--*/
{
    PCHAR AtlasIndexRaw;
    SIZE_T AtlasIndexRawSize;
    cJSON* AtlasIndex;
    cJSON* Atlas;
    cJSON* Glyphs;
    INT i;
    cJSON* GlyphData;
    cJSON* PlaneBounds;
    cJSON* AtlasBounds;
    cJSON* Object;
    WCHAR Character;
    GLYPH Glyph;
    vec4 Rectangle;
    PFONT Font;

    LogInfo("Loading font with atlas %s and atlas index %s", AtlasPath, AtlasIndexPath);

    AtlasIndexRawSize = 0;
    AtlasIndexRaw = FsReadFile(
        AtlasIndexPath,
        0,
        &AtlasIndexRawSize,
        1 // Add a NUL
        );
    if ( !AtlasIndexRaw )
    {
        LogError("Failed to read atlas index %s: %s", AtlasIndexPath, strerror(errno));
        goto Error;
    }

    AtlasIndex = cJSON_Parse(AtlasIndexRaw);
    if ( !AtlasIndex )
    {
        LogError("Failed to parse atlas index %s: %s", AtlasIndex, cJSON_GetErrorPtr());
        goto Error;
    }

    Font = PURPL_ALLOC(
        1,
        sizeof(FONT)
        );
    if ( !Font )
    {
        LogError("Failed to allocate font: %s", strerror(errno));
        goto Error;
    }

    Font->Atlas = LoadTexture(AtlasPath);
    if ( !Font->Atlas )
    {
        LogError("Failed to load texture");
        DestroyFont(Font);
        return FALSE;
    }

    Atlas = cJSON_GetObjectItemCaseSensitive(
        AtlasIndex,
        "atlas"
        );

    Glyphs = cJSON_GetObjectItemCaseSensitive(
        AtlasIndex,
        "glyphs"
        );
    if ( !Glyphs )
    {
        LogError("Font atlas index %s is missing \"glyphs\"", AtlasIndexPath);
        goto Error;
    }

    for (i = 0; i < cJSON_GetArraySize(Glyphs); i++)
    {
        memset(
            &Glyph,
            0,
            sizeof(GLYPH)
            );

        GlyphData = cJSON_GetArrayItem(
            Glyphs,
            i
            );
        Object = cJSON_GetObjectItemCaseSensitive(
            GlyphData,
            "unicode"
            );
        Character = (WCHAR)cJSON_GetNumberValue(Object);

        // Space is different, handle separately
        if (Character == L' ')
        {
            FLOAT Advance;

            Object = cJSON_GetObjectItemCaseSensitive(
                GlyphData,
                "advance"
                );
            Advance = (FLOAT)cJSON_GetNumberValue(Object);

            // (left, top, 0)
            Glyph.Corners[0][0] = 0.0f;
            Glyph.Corners[0][1] = -Advance;
            Glyph.Corners[0][2] = 0.0f;

            // (right, top, 0)
            Glyph.Corners[1][0] = Advance;
            Glyph.Corners[1][1] = -Advance;
            Glyph.Corners[1][2] = 0.0f;

            // (left, bottom, 0)
            Glyph.Corners[2][0] = 0.0f;
            Glyph.Corners[2][1] = 0.0f;
            Glyph.Corners[2][2] = 0.0f;

            // (right, bottom, 0)
            Glyph.Corners[3][0] = Advance;
            Glyph.Corners[3][1] = -Advance;
            Glyph.Corners[3][2] = 0.0f;

            memset(
                Glyph.TextureCoordinates,
                0,
                sizeof(Glyph.TextureCoordinates)
                );
        }
        else
        {
            PlaneBounds = cJSON_GetObjectItemCaseSensitive(
                GlyphData,
                "planeBounds"
                );
            Object = cJSON_GetObjectItemCaseSensitive(
                PlaneBounds,
                "left"
                );
            Rectangle[0] = (FLOAT)cJSON_GetNumberValue(Object);
            Object = cJSON_GetObjectItemCaseSensitive(
                PlaneBounds,
                "top"
                );
            Rectangle[1] = (FLOAT)cJSON_GetNumberValue(Object);
            Object = cJSON_GetObjectItemCaseSensitive(
                PlaneBounds,
                "right"
                );
            Rectangle[2] = (FLOAT)cJSON_GetNumberValue(Object);
            Object = cJSON_GetObjectItemCaseSensitive(
                PlaneBounds,
                "bottom"
                );
            Rectangle[3] = (FLOAT)cJSON_GetNumberValue(Object);

            // Vertices are made of multiple corners

            // (left, top, 0)
            Glyph.Corners[0][0] = Rectangle[0];
            Glyph.Corners[0][1] = Rectangle[1];
            Glyph.Corners[0][2] = 0.0f;

            // (right, top, 0)
            Glyph.Corners[1][0] = Rectangle[2];
            Glyph.Corners[1][1] = Rectangle[1];
            Glyph.Corners[1][2] = 0.0f;

            // (left, bottom, 0)
            Glyph.Corners[2][0] = Rectangle[0];
            Glyph.Corners[2][1] = Rectangle[3];
            Glyph.Corners[2][2] = 0.0f;

            // (right, bottom, 0)
            Glyph.Corners[3][0] = Rectangle[2];
            Glyph.Corners[3][1] = Rectangle[3];
            Glyph.Corners[3][2] = 0.0f;

            // Same with texture coordinates
            AtlasBounds = cJSON_GetObjectItemCaseSensitive(
                GlyphData,
                "atlasBounds"
                );
            Object = cJSON_GetObjectItemCaseSensitive(
                AtlasBounds,
                "left"
                );
            Rectangle[0] = (FLOAT)cJSON_GetNumberValue(Object);
            Object = cJSON_GetObjectItemCaseSensitive(
                AtlasBounds,
                "top"
                );
            Rectangle[1] = (FLOAT)cJSON_GetNumberValue(Object);
            Object = cJSON_GetObjectItemCaseSensitive(
                AtlasBounds,
                "right"
                );
            Rectangle[2] = (FLOAT)cJSON_GetNumberValue(Object);
            Object = cJSON_GetObjectItemCaseSensitive(
                AtlasBounds,
                "bottom"
                );
            Rectangle[3] = (FLOAT)cJSON_GetNumberValue(Object);

            // (left, top)
            Glyph.TextureCoordinates[0][0] = Rectangle[0];
            Glyph.TextureCoordinates[0][1] = Rectangle[1];

            // (right, top)
            Glyph.TextureCoordinates[1][0] = Rectangle[2];
            Glyph.TextureCoordinates[1][1] = Rectangle[1];

            // (left, bottom)
            Glyph.TextureCoordinates[2][0] = Rectangle[0];
            Glyph.TextureCoordinates[2][1] = Rectangle[3];

            // (right, bottom)
            Glyph.TextureCoordinates[3][0] = Rectangle[2];
            Glyph.TextureCoordinates[3][1] = Rectangle[3];
        }

        stbds_hmput(
            Font->Glyphs,
            Character,
            Glyph
            );
    }

Error:
    if ( AtlasIndexRaw )
    {
        PURPL_FREE(AtlasIndexRaw);
    }

    if ( AtlasIndex )
    {
        cJSON_Delete(AtlasIndex);
    }

    return Font;
}

VOID
DestroyFont(
    _In_opt_ PFONT Font
    )
/*++

Routine Description:

    Destroys a font.

Arguments:

    Font - The font to destroy.

Return Value:

    None.

--*/
{
    if ( !Font )
    {
        return;
    }

    if ( Font->Atlas )
    {
        PURPL_FREE(Font->Atlas);
    }

    if ( Font->Glyphs )
    {
        stbds_hmfree(Font->Glyphs);
    }

    PURPL_FREE(Font);
}