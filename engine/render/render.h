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

#include "engine/math/transform.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"

/// @brief Graphics API
PURPL_MAKE_TAG(enum, RENDER_API,
               {RenderApiVulkan, RenderApiDirect3D12, RenderApiDirect3D9, RenderApiOpenGL, RenderApiSwRaster,
                RenderApiCount})

/// @brief A handle to a renderer backend thing (Vulkan always uses 64-bit handles)
typedef UINT64 RENDER_HANDLE;

/// @brief Material
PURPL_MAKE_TAG(struct, MATERIAL, {
    RENDER_HANDLE Handle;
    RENDER_HANDLE TextureHandle;
    RENDER_HANDLE ShaderHandle;
})

/// @brief Model component, stores backend handles
PURPL_MAKE_COMPONENT(struct, MODEL, {
    RENDER_HANDLE MeshHandle;
    PMATERIAL Material;
})

/// @brief Maximum number of models
#define RENDER_MAX_MODEL_COUNT 1024

/// @brief Maximum number of textures
#define RENDER_MAX_TEXTURE_COUNT 1024

/// @brief Maximum number of objects
#define RENDER_MAX_OBJECT_COUNT 1024

/// @brief Maximum number of vertices when using RdrDrawGeometry
///
/// If you have more vertices than this, there's probably no reason not to use a model.
#define RENDER_MAX_GEOMETRY_SIZE 576

/// @brief Shader spaces
#define RENDER_SHADER_SPACE_SCENE 0
#define RENDER_SHADER_SPACE_OBJECT 1

/// @brief Shader registers

#define RENDER_SHADER_SCENE_UBO_REGISTER 0
#define RENDER_SHADER_OBJECT_UBO_REGISTER 1
#define RENDER_SHADER_SAMPLER_REGISTER 2

/// @brief Uniform data for the whole scene
PURPL_MAKE_TAG(struct, RENDER_SCENE_UNIFORM, {
    vec3 CameraPosition;
    mat4 View;
    mat4 Projection;
})

/// @brief Uniform data for individual objects
PURPL_MAKE_TAG(struct, RENDER_OBJECT_UNIFORM, { mat4 Model; })

PURPL_MAKE_COMPONENT(struct, RENDER_OBJECT_DATA, { RENDER_HANDLE Handle; })

/// @brief Renderer backend
PURPL_MAKE_TAG(struct, RENDER_BACKEND, {
    PCSTR Name;

    VOID (*Initialize)(VOID);
    VOID (*BeginFrame)(_In_ BOOLEAN WindowResized, _In_ PRENDER_SCENE_UNIFORM Uniform);
    VOID (*EndFrame)(VOID);
    VOID (*FinishRendering)(VOID);
    VOID (*Shutdown)(VOID);

    RENDER_HANDLE (*LoadShader)(_In_z_ PCSTR Name);
    VOID (*DestroyShader)(_In_ RENDER_HANDLE Handle);

    RENDER_HANDLE (*UseTexture)(_In_ PTEXTURE Texture, _In_z_ PCSTR Name);
    VOID (*ReleaseTexture)(_In_ RENDER_HANDLE Handle);

    VOID (*CreateMaterial)(_Inout_ PMATERIAL Material);
    VOID (*DestroyMaterial)(_In_ PMATERIAL Material);

    VOID (*CreateModel)(_In_z_ PCSTR Name, _Inout_ PMODEL Model, _In_ CONST PMESH Mesh);
    VOID(*DrawModel)
    (_In_ PMODEL Model, _In_ CONST PRENDER_OBJECT_UNIFORM Uniform, _In_ CONST PRENDER_OBJECT_DATA Data);
    VOID (*DestroyModel)(_Inout_ PMODEL Model);

    VOID(*DrawGeometry)
    (_In_ PCMESH_VERTEX Vertices, _In_ SIZE_T VertexCount, _In_opt_ CONST ivec3 * CONST Indices,
     _In_ SIZE_T IndexCount, _In_ RENDER_HANDLE Shader, _In_opt_ mat4 CONST Transform, _In_ BOOLEAN Project);

    VOID (*InitializeObject)(_In_z_ PCSTR Name, _Inout_ PRENDER_OBJECT_DATA Data, _In_ PMODEL Model);
    VOID (*DestroyObject)(_Inout_ PRENDER_OBJECT_DATA Data);

    PCSTR (*GetGpuName)(VOID);
})

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
/// @param[in] Name The name of the texture to load
///
/// @return A texture handle.
extern RENDER_HANDLE RdrLoadTexture(_In_z_ PCSTR Name);

/// @brief Release a texture
///
/// @param[in] TextureHandle The texture handle to release
extern VOID RdrDestroyTexture(_In_ RENDER_HANDLE TextureHandle);

/// @brief Create a material
///
/// @param[out] Material The material to create
/// @param[in] TextureHandle The texture to use
/// @param[in] ShaderName The name of the shader program to use
///
/// @return A material. Must not outlive the texture handle.
extern BOOLEAN RdrCreateMaterial(_Out_ PMATERIAL Material, _In_ RENDER_HANDLE TextureHandle, _In_z_ PCSTR ShaderName);

/// @brief Destroy a material
///
/// @param[in] Material The material to destroy
extern VOID RdrDestroyMaterial(_In_ PMATERIAL Material);

/// @brief Create a model
///
/// @param[out] Model The model to create
/// @param[in] Name The name of the mesh to use
/// @param[in] Material The material to use
///
/// @return A model. This must not outlive the source mesh.
extern BOOLEAN RdrLoadModel(_Out_ PMODEL Model, _In_z_ PCSTR Name, _In_ PMATERIAL Material);

/// @brief Destroy a model
///
/// @param Model The model to destroy
extern VOID RdrDestroyModel(_In_ PMODEL Model);

/// @brief Initialize per-object data
///
/// @param[in] Name The name of the object
/// @param[in,out] Data The per-object data to initialize
/// @param[in] Model The model used by this object
extern VOID RdrInitializeObject(_In_z_ PCSTR Name, _Inout_ PRENDER_OBJECT_DATA Data, _In_ PMODEL Model);

/// @brief Destroy per-object data
///
/// @param[in,out] Data The per-object data to destroy
extern VOID RdrDestroyObject(_Inout_ PRENDER_OBJECT_DATA Data);

/// @brief Tell the renderer that it's done
extern VOID RdrFinishRendering(VOID);

/// @brief Shut down the render system
extern VOID RdrShutdown(VOID);

/// @brief Draw geometry
extern VOID RdrDrawGeometry(_In_ PCMESH_VERTEX Vertices, _In_ SIZE_T VertexCount, _In_opt_ ivec3 *Indices,
                            _In_ SIZE_T IndexCount, _In_opt_ PMATERIAL Material, _In_opt_ mat4 Transform,
                            _In_ BOOLEAN Project);

/// @brief Draw a line
extern VOID RdrDrawLine(_In_ vec3 Start, _In_ vec3 End, _In_ vec4 Colour, _In_opt_ mat4 Transform,
                        _In_ BOOLEAN Project);

extern VOID RdrDrawRectangle(_In_ PCMESH_VERTEX Vertices, _In_opt_ PMATERIAL Material, _In_opt_ mat4 Transform);

/// @brief Get the width of the render output
///
/// @return The scaled width of the render output
extern UINT32 RdrGetWidth(VOID);

/// @brief Get the height of the render output
///
/// @return The scaled height of the render output
extern UINT32 RdrGetHeight(VOID);

/// @brief Get the name of a render API
///
/// @return A string representing the render API
extern PCSTR RdrGetApiName(_In_ RENDER_API Api);

/// @brief Get the name of the current GPU
///
/// @return The name of the current GPU that will remain valid until RdrShutdown
extern PCSTR RdrGetGpuName(VOID);
