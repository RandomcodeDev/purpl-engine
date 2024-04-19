/// @file opengl.h
///
/// @brief This file contains definitions used by the OpenGL backend
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/alloc.h"
#include "common/common.h"
#include "common/log.h"

#include "engine/engine.h"

#include "engine/render/render.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"

/// @brief Data for a model
typedef struct OPENGL_MODEL_DATA
{
    UINT32 VertexBuffer;
    UINT32 IndexBuffer;
    UINT32 VertexArray;
    UINT32 ElementCount;
} OPENGL_MODEL_DATA, *POPENGL_MODEL_DATA;

/// @brief Global OpenGL stuff
typedef struct OPENGL_DATA
{
    UINT32 UniformBufferAlignment;
    UINT32 UniformBuffer;
} OPENGL_DATA, *POPENGL_DATA;

extern OPENGL_DATA GlData;

/// @brief Load a shader
///
/// @param[in] Name The name of the shader to load
///
/// @return The handle to the shader
extern UINT64 GlLoadShader(_In_ PCSTR Name);

/// @brief Destroy a shader
///
/// @param[in] Shader The handle to the shader to destroy
extern VOID GlDestroyShader(_In_ UINT64 Shader);

/// @brief Create a uniform buffer
extern UINT32 GlCreateUniformBuffer(UINT32 Size);

/// @brief Write data to a uniform buffer
extern VOID GlWriteUniformBuffer(UINT32 UniformBuffer, UINT32 Offset, PVOID Data, UINT32 Size);

/// @brief Create a model from a mesh
///
/// @param[in,out] Model The model to create
/// @param[in] Mesh The mesh to use
extern VOID GlCreateModel(_Inout_ PMODEL Model, _In_ PMESH Mesh);

/// @brief Draw a model
///
/// @param[in] Model The model to render
/// @param[in] Uniform The per-object uniform data for rendering the object
extern VOID GlDrawModel(_In_ PMODEL Model, _In_ PRENDER_OBJECT_UNIFORM Uniform);

/// @brief Destroy a model
///
/// @param[in,out] The model to destroy
extern VOID GlDestroyModel(_Inout_ PMODEL Model);

/// @brief Set the debug callback
extern VOID GlSetDebugCallback(VOID);
