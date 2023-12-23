/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    rendersystem.h

Abstract:

    This module defines the render system API.

    TODO: Make textures, meshes, fonts, etc be allocated from a pool or something to reduce fragmentation
    TODO: Add multi-viewport (splitscreen) rendering support in a cross-API way

--*/

#pragma once

#include "purpl/purpl.h"

#include "engine/mathutil.h"

#ifdef PURPL_DIRECTX
#include "directx/dx.h"
#endif

#ifdef PURPL_DIRECTX9
#include "directx/dx9.h"
#endif

#ifdef PURPL_VULKAN
#include "vulkan/vk.h"
#endif

#include "util/font.h"
#include "util/mesh.h"
#include "util/texture.h"

//
// Render APIs
//

typedef enum RENDER_API
{
    RenderApiNone,
    RenderApiDX9,
    RenderApiDX12,
    RenderApiVulkan,
    RenderApiCount
} RENDER_API, *PRENDER_API;

//
// Current render API
//

extern RENDER_API RenderApi;

//
// Number of frames rendered
//

extern UINT64 RenderedFrames;

//
// Shader type, controls vertex input format
//

typedef enum SHADER_TYPE
{
    ShaderTypeMesh,
    ShaderTypeFont,
    ShaderTypeUi,
    ShaderTypePostProcess
} SHADER_TYPE, *PSHADER_TYPE;

//
// Shader
//

typedef struct SHADER
{
    CHAR Name[32];

    PVOID VertexData;
    SIZE_T VertexLength;
    PVOID FragmentData;
    SIZE_T FragmentLength;

    SHADER_TYPE Type;

    PVOID Handle;
} SHADER, *PSHADER;

typedef struct RENDER_GLOBAL_UNIFORM_DATA
{
    mat4 ViewTransform;
    mat4 ProjectionTransform;
} RENDER_GLOBAL_UNIFORM_DATA, *PRENDER_GLOBAL_UNIFORM_DATA;

typedef struct RENDER_MODEL_UNIFORM_DATA
{
    mat4 ModelTransform;
} RENDER_MODEL_UNIFORM_DATA, *PRENDER_MODEL_UNIFORM_DATA;

typedef struct RENDER_FONT_UNIFORM_DATA
{
    mat4 Transform;
    vec4 Colour;
} RENDER_FONT_UNIFORM_DATA, *PRENDER_FONT_UNIFORM_DATA;

typedef struct SHADER_MAP
{
    PCHAR key;
    PSHADER value;
} SHADER_MAP, *PSHADER_MAP;

//
// Render system texture
//

typedef struct RENDER_TEXTURE
{
    CHAR Name[32];
    PTEXTURE Texture;
    PVOID Handle;
} RENDER_TEXTURE, *PRENDER_TEXTURE;

typedef struct RENDER_TEXTURE_MAP
{
    PCHAR key;
    PRENDER_TEXTURE value;
} RENDER_TEXTURE_MAP, *PRENDER_TEXTURE_MAP;

//
// Material, binds a shader and texture
//

typedef struct MATERIAL
{
    CHAR Name[32];
    PSHADER Shader;
    PRENDER_TEXTURE Texture;
} MATERIAL, *PMATERIAL;

typedef struct MATERIAL_MAP
{
    PCHAR key;
    PMATERIAL value;
} MATERIAL_MAP, *PMATERIAL_MAP;

//
// Render system font
//

typedef struct RENDER_FONT
{
    CHAR Name[32];
    PFONT Font;
    PRENDER_TEXTURE Atlas;
    PVOID Handle;
} RENDER_FONT, *PRENDER_FONT;

typedef struct RENDER_FONT_MAP
{
    PCHAR key;
    PRENDER_FONT value;
} RENDER_FONT_MAP, *PRENDER_FONT_MAP;

//
// Model
//

typedef struct MODEL
{
    CHAR Name[32];
    PMESH Mesh;
    PMATERIAL Material;

    PVOID Handle;
} MODEL, *PMODEL;

typedef struct MODEL_MAP
{
    PCHAR key;
    PMODEL value;
} MODEL_MAP, *PMODEL_MAP;

//
// Type of renderable thing
//

typedef enum RENDERABLE_TYPE
{
    RenderableTypeNone,
    RenderableTypeModel,
    RenderableTypeCount
} RENDERABLE_TYPE, *PRENDERABLE_TYPE;

//
// A thing that can be rendered
//

typedef struct RENDERABLE
{
    RENDERABLE_TYPE Type;
    union {
        PMODEL Model;
    };
} RENDERABLE, *PRENDERABLE;
extern ECS_COMPONENT_DECLARE(RENDERABLE);

#ifdef RENDERSYSTEM_IMPLEMENTATION
//
// Shaders by name
//

extern PSHADER_MAP Shaders;

//
// Textures by name
//

extern PRENDER_TEXTURE_MAP Textures;

//
// Materials by name
//

extern PMATERIAL_MAP Materials;

//
// Fonts by name
//

extern PRENDER_FONT_MAP Fonts;

//
// Models by name
//

extern PMODEL_MAP Models;
#endif

//
// Render system interface. See the Render* functions for descriptions.
//

typedef struct RENDER_SYSTEM_INTERFACE
{
    RENDER_API Api;
    PCSTR Name;
    PCSTR DeviceName;
    SIZE_T DeviceIndex;
    UINT16 DeviceVendorId;
    UINT16 DeviceId;

    VOID
    (*Initialize)(
        VOID
        );
    VOID
    (*BeginCommands)(
        _In_ PRENDER_GLOBAL_UNIFORM_DATA UniformData,
        _In_ BOOLEAN WindowResized
        );
    VOID
    (*DrawModel)(
        _In_ PMODEL Model,
        _In_ PRENDER_MODEL_UNIFORM_DATA UniformData
        );
    VOID
    (*Present)(
        VOID
        );
    VOID
    (*Shutdown)(
        VOID
        );

    VOID
    (*CreateShader)(
        _In_ PSHADER SourceShader
        );
    VOID
    (*DestroyShader)(
        _In_ PSHADER Shader
        );

    PMODEL
    (*LoadModel)(
        _In_ PCSTR Path
        );
    VOID
    (*UseMesh)(
        _In_ PMODEL SourceModel
        );
    VOID
    (*DestroyModel)(
        _In_ PMODEL Model
        );

    VOID
    (*UseTexture)(
        _In_ PRENDER_TEXTURE SourceTexture
        );
    PRENDER_TEXTURE
    (*LoadTexture)(
        _In_ PCSTR Path
        );
    VOID
    (*DestroyTexture)(
        _In_ PRENDER_TEXTURE Texture
        );

    VOID
    (*UseFont)(
        _In_ PRENDER_FONT SourceFont
        );
    VOID
    (*DestroyFont)(
        _In_ PRENDER_FONT Font
        );

    VOID
    (*DrawGlyph)(
        _In_ PRENDER_FONT Font,
        _In_ PRENDER_FONT_UNIFORM_DATA UniformData,
        _In_ PGLYPH Glyph,
        _In_ SIZE_T Offset
        );
} RENDER_SYSTEM_INTERFACE, *PRENDER_SYSTEM_INTERFACE;

extern RENDER_SYSTEM_INTERFACE RenderInterfaces[RenderApiCount];
extern RENDER_API RenderApi;

//
// Initialize the render system
//

extern
VOID
RenderInitialize(
    VOID
    );

//
// Set up to record render commands
//

extern
VOID
RenderBeginCommands(
    VOID
    );

//
// Finish and present the frame
//

extern
VOID
RenderPresentFrame(
    VOID
    );

//
// Clean up render system
//

extern
VOID
RenderShutdown(
    VOID
    );

//
// Create a shader
//

extern
BOOLEAN
RenderCreateShader(
    _In_ PCSTR Name,
    _In_reads_(VertexLength) PVOID VertexData,
    _In_ SIZE_T VertexLength,
    _In_reads_(FragmentLength) PVOID FragmentData,
    _In_ SIZE_T FragmentLength,
    _In_ SHADER_TYPE ShaderType
    );

//
// Load a shader
//

extern
BOOLEAN
RenderLoadShader(
    _In_ PCSTR Name,
    _In_ SHADER_TYPE ShaderType
    );

//
// Destroy a shader
//

extern
VOID
RenderDestroyShader(
    _In_opt_ PCSTR Name
    );

//
// Get a texture
//

extern
PSHADER
RenderGetShader(
    _In_opt_ PCSTR Name
    );

//
// Create a model
//

extern
BOOLEAN
RenderCreateModel(
    _In_ PCSTR Name,
    _In_ PCSTR Material,
    _In_reads_(VertexCount * sizeof(VERTEX)) PVERTEX Vertices,
    _In_ SIZE_T VertexCount,
    _In_reads_(IndexCount * sizeof(ivec3)) ivec3* Indices,
    _In_ SIZE_T IndexCount
    );

//
// Load a model, making it accessible as "<Name>-<Index>"
//

extern
BOOLEAN
RenderLoadModel(
    _In_ PCSTR Name,
    _In_ UINT32 Index
    );

//
// Create a model from a mesh
//

extern
BOOLEAN
RenderUseMesh(
    _In_ PCSTR Name,
    _In_ PMESH Mesh
    );

//
// Destroy a model
//

extern
VOID
RenderDestroyModel(
    _In_opt_ PCSTR Name
    );

//
// Get a model
//

extern
PMODEL
RenderGetModel(
    _In_opt_ PCSTR Name
    );

//
// Create a texture
//

extern
BOOLEAN
RenderCreateTexture(
    _In_ PCSTR Name,
    _In_ UINT32 Width,
    _In_ UINT32 Height,
    _In_ TEXTURE_FORMAT Format,
    _In_ PVOID Data
    );

//
// Load a texture
//

extern
BOOLEAN
RenderLoadTexture(
    _In_ PCSTR Name
    );

//
// Create a render texture from a normal texture
//

extern
BOOLEAN
RenderUseTexture(
    _In_ PCSTR Name,
    _In_ PTEXTURE Texture
    );

//
// Destroy a texture
//

extern
VOID
RenderDestroyTexture(
    _In_opt_ PCSTR Name
    );

//
// Get a texture
//

extern
PRENDER_TEXTURE
RenderGetTexture(
    _In_opt_ PCSTR Name
    );

//
// Add a material
//

extern
VOID
RenderAddMaterial(
    _In_ PCSTR Name,
    _In_ PCSTR ShaderName,
    _In_ PCSTR TextureName
    );

//
// Delete a material
//

extern
VOID
RenderDeleteMaterial(
    _In_opt_ PCSTR Name
    );

//
// Get a material
//

extern
PMATERIAL
RenderGetMaterial(
    _In_opt_ PCSTR Name
    );

//
// Load a font
//

extern
BOOLEAN
RenderLoadFont(
    _In_ PCSTR Name
    );

//
// Use a font
//

extern
BOOLEAN
RenderUseFont(
    _In_ PCSTR Name,
    _In_ PFONT Font
    );

//
// Destroy a font
//

extern
VOID
RenderDestroyFont(
    _In_opt_ PCSTR Name
    );

//
// Get a font
//

extern
PRENDER_FONT
RenderGetFont(
    _In_opt_ PCSTR Name
    );

//
// Options for drawing text
//

typedef struct RENDER_TEXT_OPTIONS
{
    vec2 TopLeft;
    FLOAT Scale;
    vec4 Colour;
    vec2 Padding;

    // TODO: implement these
    BOOLEAN Wrap;
    BOOLEAN CutOff;
    vec2 BottomRight;
} RENDER_TEXT_OPTIONS, *PRENDER_TEXT_OPTIONS;

//
// Draw a character
//

extern
FLOAT
RenderDrawCharacter(
    _In_ PCSTR FontName,
    _In_ FLOAT Scale,
    _In_ vec4 Colour,
    _In_ vec2 Position,
    _In_ WCHAR Character
    );

//
// Draw a string
// TODO: Add way more options, maybe a struct?
//

extern
VOID
RenderDrawString(
    _In_ PCSTR FontName,
    _In_ PRENDER_TEXT_OPTIONS Options,
    _In_ PCSTR Format,
    ...
    );

//
// Draw the UI
//

extern
VOID
RenderDrawUi(
    VOID
    );

//
// Draw an entity
//

extern
VOID
RenderDrawEntity(
    _In_ ecs_iter_t* Entity
    );
extern ECS_SYSTEM_DECLARE(RenderDrawEntity);
