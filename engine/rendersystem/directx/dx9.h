/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    dx9.h

Abstract:

    This module defines the DirectX 9 render interface.

--*/

#pragma once

#ifdef PURPL_DEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>
//#ifdef PURPL_DEBUG
//#include <d3d9on12.h>
//#endif

#include "purpl/purpl.h"

BEGIN_EXTERN_C

#include "common/common.h"
#include "common/log.h"

#include "util/mesh/mesh.h"

#if defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO
#define PURPL_DIRECTX9_DEBUG 1
#endif

//
// Struct declarations
//

typedef struct RENDERABLE RENDERABLE, *PRENDERABLE;
typedef struct RENDER_GLOBAL_UNIFORM_DATA RENDER_GLOBAL_UNIFORM_DATA, *PRENDER_GLOBAL_UNIFORM_DATA;
typedef struct RENDER_MODEL_UNIFORM_DATA RENDER_MODEL_UNIFORM_DATA, *PRENDER_MODEL_UNIFORM_DATA;
typedef struct SHADER SHADER, *PSHADER;
typedef struct MODEL MODEL, *PMODEL;
typedef struct RENDER_TEXTURE RENDER_TEXTURE, *PRENDER_TEXTURE;

//
// Initialize DirectX 9
//

extern
VOID
DirectX9Initialize(
    VOID
    );

//
// Prepare to start a frame
//

extern
VOID
DirectX9BeginCommands(
    _In_ PRENDER_GLOBAL_UNIFORM_DATA UniformData
    );

//
// Draw a model
//

extern
VOID
DirectX9DrawModel(
    _In_ PMODEL Model,
    _In_ PRENDER_MODEL_UNIFORM_DATA UniformData
    );

//
// Present the rendered frame
//

extern
VOID
DirectX9PresentFrame(
    VOID
    );

//
// Shut down DirectX 9
//

extern
VOID
DirectX9Shutdown(
    VOID
    );

//
// Create a shader
//

extern
VOID
DirectX9CreateShader(
    _In_ PSHADER SourceShader
    );

//
// Destroy a shader
//

extern
VOID
DirectX9DestroyShader(
    _In_ PSHADER Shader
    );

//
// Use a model
//

extern
VOID
DirectX9UseMesh(
    _In_ PMODEL SourceModel
    );

//
// Destroy a model
//

extern
VOID
DirectX9DestroyModel(
    _In_ PMODEL Model
    );

//
// Use a texture
//

extern
VOID
DirectX9UseTexture(
    _In_ PRENDER_TEXTURE Texture
    );

//
// Destroy a texture
//

extern
VOID
DirectX9DestroyTexture(
    _In_ PRENDER_TEXTURE Texture
    );

END_EXTERN_C
