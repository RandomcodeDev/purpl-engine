/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    font.h

Abstract:

    This module defines the font API.

--*/

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"

#include "texture.h"

//
// A glyph of a font
//

typedef struct GLYPH
{
    vec3 Corners[4];
    vec2 TextureCoordinates[4];
} GLYPH, *PGLYPH;

//
// Maps characters to glyphs
//

typedef struct GLYPH_MAP
{
    WCHAR key; // Character
    GLYPH value; // Glyph
} GLYPH_MAP, *PGLYPH_MAP;

//
// A font
//

typedef struct FONT
{
    PTEXTURE Atlas;
    UINT32 GlyphSize;
    PGLYPH_MAP Glyphs;
} FONT, *PFONT;

//
// Load a font
//

extern
PFONT
LoadFont(
    _In_ PCSTR AtlasPath,
    _In_ PCSTR AtlasIndexPath
    );

//
// Destroy a font
//

extern
VOID
DestroyFont(
    _In_opt_ PFONT Font
    );
