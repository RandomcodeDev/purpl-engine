/// @file render.h
///
/// @brief This file declares the render API.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/configvar.h"
#include "common/log.h"

#include "engine/mathutil.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"

/// @brief Graphics API
typedef enum RENDER_API
{
    RenderApiNone,
    RenderApiVulkan,
    RenderApiDirect3D12,
    RenderApiCount
} RENDER_API, *PRENDER_API;

/// @brief Material
typedef struct MATERIAL
{
    PVOID Handle;
    PVOID TextureHandle;
    PCSTR ShaderName;
} MATERIAL, *PMATERIAL;

/// @brief Model component, stores backend handles
typedef struct MODEL
{
    PVOID MeshHandle;
    PMATERIAL Material;
} MODEL, *PMODEL;
extern ECS_COMPONENT_DECLARE(MODEL);

/// @brief Renderer backend
typedef struct RENDER_BACKEND
{
    VOID (*Initialize)(VOID);
    VOID (*BeginFrame)(_In_ BOOLEAN WindowResized);
    VOID (*EndFrame)(VOID);
    VOID (*Shutdown)(VOID);

    PVOID (*LoadShader)(_In_ PCSTR Name);
    VOID (*DestroyShader)(_In_ PVOID Handle);

    PVOID (*UseTexture)(_In_ PTEXTURE Texture);
    VOID (*ReleaseTexture)(_In_ PVOID TextureHandle);

    VOID (*CreateMaterial)(_Inout_ PMATERIAL Material);
    VOID (*DestroyMaterial)(_In_ PMATERIAL Material);

    VOID (*CreateModel)(_Inout_ PMODEL Model, _In_ PMESH Mesh);
    VOID (*DrawModel)(_In_ PMODEL Model, _In_ mat4 Object, _In_ mat4 World, _In_ mat4 Projection);
    VOID (*DestroyModel)(_In_ PMODEL Model);
} RENDER_BACKEND, *PRENDER_BACKEND;

/// @brief Define configuration variables
extern VOID RdrDefineVariables(VOID);

/// @brief Initialize the render system
extern ECS_SYSTEM_DECLARE(RdrInitialize);

/// @brief Start recording a frame
extern ECS_SYSTEM_DECLARE(RdrBeginFrame);

/// @brief Draw a model
extern ECS_SYSTEM_DECLARE(RdrDrawModel);

/// @brief Finish recording a frame and present it
extern ECS_SYSTEM_DECLARE(RdrEndFrame);

/// @brief Use a texture
///
/// @param[in] Texture The texture to use
///
/// @return A texture handle. Must not outlive the source texture.
extern PVOID RdrUseTexture(_In_ PTEXTURE Texture);

/// @brief Release a texture
///
/// @param[in] TextureHandle The texture handle to release
extern VOID RdrReleaseTexture(_In_ PVOID TextureHandle);

/// @brief Create a material
///
/// @param[out] Material The material to create
/// @param[in] Texture The texture to use
/// @param[in] ShaderName The name of the shader program to use
///
/// @return A material. Must not outlive the texture handle.
extern BOOLEAN RdrCreateMaterial(_Out_ PMATERIAL Material, _In_ PVOID TextureHandle, _In_z_ PCSTR ShaderName);

/// @brief Destroy a material
///
/// @param[in] Material The material to destroy
extern VOID RdrDestroyMaterial(_In_ PMATERIAL Material);

/// @brief Create a model
///
/// @param[out] Model The model to create
/// @param[in] Mesh The mesh to use
/// @param[in] Material The material to use
///
/// @return A model. This must not outlive the source mesh.
extern BOOLEAN RdrCreateModel(_Out_ PMODEL Model, _In_ PMESH Mesh, _In_ PMATERIAL Material);

/// @brief Destroy a model
///
/// @param Model The model to destroy
extern VOID RdrDestroyModel(_In_ PMODEL Model);

/// @brief Shut down the render system
extern VOID RdrShutdown(VOID);

/// @brief Get the width of the render output
///
/// @return The scaled width of the render output
extern UINT32 RdrGetWidth(VOID);

/// @brief Get the height of the render output
///
/// @return The scaled height of the render output
extern UINT32 RdrGetHeight(VOID);
