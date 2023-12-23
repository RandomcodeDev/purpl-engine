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
// A vertex of a glyph
//

typedef struct GLYPH_VERTEX
{
    vec2 Position;
    vec2 TextureCoordinate;
} GLYPH_VERTEX, *PGLYPH_VERTEX;

//
// A glyph of a font
//

typedef struct GLYPH
{
    GLYPH_VERTEX Corners[4];
    FLOAT Width;
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
