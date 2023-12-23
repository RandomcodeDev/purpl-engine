/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    rendersystem.c

Abstract:

    This module implements the API-independant parts of the render system.

--*/

#include "engine/camera.h"
#include "engine/engine.h"
#include "engine/entity.h"

#include "rendersystem.h"

RENDER_API RenderApi;
UINT64 RenderedFrames;
static BOOLEAN InFrame;

#define ASSETS_PREFIX SWITCH_ROMFS_MOUNTPOINT "assets/"

RENDER_SYSTEM_INTERFACE RenderInterfaces[RenderApiCount] =
{
    {
        RenderApiNone,
        "None",

        .Initialize = NULL,
        .BeginCommands = NULL,

        .Present = NULL,
        .Shutdown = NULL,

        .CreateShader = NULL,
        .DestroyShader = NULL,

        .LoadModel = NULL,
        .UseMesh = NULL,
        .DestroyModel = NULL,

        .LoadTexture = NULL,
        .UseTexture = NULL,
        .DestroyTexture = NULL,

        .DrawModel = NULL,
    },
    {
        RenderApiDX12,
        "Direct3D 12",

#ifdef PURPL_DIRECTX
        .Initialize = NULL,
        .BeginCommands = NULL,

        .Present = NULL,
        .Shutdown = NULL,

        .CreateShader = NULL,
        .DestroyShader = NULL,

        .LoadModel = NULL,
        .UseMesh = NULL,
        .DestroyModel = NULL,

        .LoadTexture = NULL,
        .UseTexture = NULL,
        .DestroyTexture = NULL,

        .DrawModel = NULL,
#endif
    },
    {
        RenderApiDX9,
        "Direct3D 9",

#ifdef PURPL_DIRECTX9
        .Initialize = DirectX9Initialize,
        .BeginCommands = DirectX9BeginCommands,

        .Present = DirectX9PresentFrame,
        .Shutdown = DirectX9Shutdown,

        .CreateShader = DirectX9CreateShader,
        .DestroyShader = DirectX9DestroyShader,

        .LoadModel = NULL,
        .UseMesh = DirectX9UseMesh,
        .DestroyModel = DirectX9DestroyModel,

        .LoadTexture = NULL,
        .UseTexture = DirectX9UseTexture,
        .DestroyTexture = DirectX9DestroyTexture,

        .DrawModel = DirectX9DrawModel,
#endif
    },
    {
        RenderApiVulkan,
        "Vulkan 1.3",

#ifdef PURPL_VULKAN
        .Initialize = VulkanInitialize,
        .BeginCommands = VulkanBeginCommands,

        .Present = VulkanPresentFrame,
        .Shutdown = VulkanShutdown,

        .CreateShader = VulkanCreateShader,
        .DestroyShader = VulkanDestroyShader,

        .LoadModel = NULL,
        .UseMesh = VulkanUseMesh,
        .DestroyModel = VulkanDestroyModel,

        .LoadTexture = NULL,
        .UseTexture = VulkanUseTexture,
        .DestroyTexture = VulkanDestroyTexture,

        .DrawModel = VulkanDrawModel,
#endif
    }
};

PSHADER_MAP Shaders;
PRENDER_TEXTURE_MAP Textures;
PMATERIAL_MAP Materials;
PMODEL_MAP Models;

VOID
RenderInitialize(
    VOID
    )
/*++

Routine Description:

    Initializes rendering.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogInfo("Initializing render system");

#ifdef PURPL_VULKAN
    RenderApi = RenderApiVulkan;
#elif defined(PURPL_DIRECTX)
    RenderApi = RenderApiDX12;
#elif defined(PURPL_DIRECTX9)
    RenderApi = RenderApiDX9;
#endif

    LogTrace("RenderInterfaces[%s] =", RenderInterfaces[RenderApi]);
    LogTrace("{");
    LogTrace("\t.Initialize = 0x%llX,", RenderInterfaces[RenderApi].Initialize);
    LogTrace("\t.BeginCommands = 0x%llX,", RenderInterfaces[RenderApi].BeginCommands);
    LogTrace("\t.Present = 0x%llX,", RenderInterfaces[RenderApi].Present);
    LogTrace("\t.Shutdown = 0x%llX,", RenderInterfaces[RenderApi].Shutdown);
    LogTrace("\t.CreateShader = 0x%llX,", RenderInterfaces[RenderApi].CreateShader);
    LogTrace("\t.DestroyShader = 0x%llX,", RenderInterfaces[RenderApi].DestroyShader);
    LogTrace("\t.LoadModel = 0x%llX,", RenderInterfaces[RenderApi].LoadModel);
    LogTrace("\t.UseMesh = 0x%llX,", RenderInterfaces[RenderApi].UseMesh);
    LogTrace("\t.DestroyModel = 0x%llX,", RenderInterfaces[RenderApi].DestroyModel);
    LogTrace("\t.LoadTexture = 0x%llX,", RenderInterfaces[RenderApi].LoadTexture);
    LogTrace("\t.UseTexture = 0x%llX,", RenderInterfaces[RenderApi].UseTexture);
    LogTrace("\t.DestroyTexture = 0x%llX,", RenderInterfaces[RenderApi].DestroyTexture);
    LogTrace("\t.DrawModel = 0x%llX,", RenderInterfaces[RenderApi].DrawModel);
    LogTrace("}");

    if ( RenderInterfaces[RenderApi].Initialize )
    {
        LogTrace("Calling RenderInterfaces[%s].Initialize()", RenderInterfaces[RenderApi].Name);
        RenderInterfaces[RenderApi].Initialize();
    }

    RenderedFrames = 0;

    LogInfo("Render system initialization succeeded");
}

VOID
RenderBeginCommands(
    VOID
    )
/*++

Routine Description:

    Sets up command structures to render a frame.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 Width;
    UINT32 Height;
    DOUBLE Aspect;
    PCAMERA MainCamera;

    PlatformGetVideoSize(
        &Width,
        &Height
        );
    Aspect = (DOUBLE)Width / (DOUBLE)Height;
    MainCamera = ecs_get_mut(
        EngineGetEcsWorld(),
        EngineGetMainCamera(),
        CAMERA
        );
    MainCamera->Changed = MainCamera->Aspect != Aspect;
    MainCamera->Aspect = Aspect;
    CalculateCameraMatrices(MainCamera);

    RENDER_GLOBAL_UNIFORM_DATA UniformData = {0};
    glm_mat4_copy(
        MainCamera->View,
        UniformData.ViewTransform
        );
    glm_mat4_copy(
        MainCamera->Projection,
        UniformData.ProjectionTransform
        );

    InFrame = TRUE;
    if ( RenderInterfaces[RenderApi].BeginCommands )
    {
        RenderInterfaces[RenderApi].BeginCommands(
            &UniformData,
            PlatformVideoResized()
            );
    }
}

VOID
RenderPresentFrame(
    VOID
    )
/*++

Routine Description:

    Presents the last rendered frame.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if ( RenderInterfaces[RenderApi].Present )
    {
        RenderInterfaces[RenderApi].Present();
    }
    RenderedFrames++;
    InFrame = FALSE;
}

VOID
RenderShutdown(
    VOID
    )
/*++

Routine Description:

    Shuts down rendering.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogInfo("Shutting down render system, %llu frames rendered", RenderedFrames);

    if ( RenderInterfaces[RenderApi].Shutdown )
    {
        LogTrace("Calling RenderInterfaces[%s].Shutdown()", RenderInterfaces[RenderApi].Name);
        RenderInterfaces[RenderApi].Shutdown();
    }

    stbds_shfree(Models);
    stbds_shfree(Materials);
    stbds_shfree(Shaders);
    stbds_shfree(Textures);

    LogInfo("Render system shutdown succeeded");
}

BOOLEAN
RenderCreateShader(
    _In_ PCSTR Name,
    _In_reads_(VertexLength) PVOID VertexData,
    _In_ SIZE_T VertexLength,
    _In_reads_(FragmentLength) PVOID FragmentData,
    _In_ SIZE_T FragmentLength,
    _In_ SHADER_TYPE ShaderType
    )
/*++

Routine Description:

    Creates a shader from loaded data.

Arguments:

    Name - The name of the shader.

    VertexData - The contents of the vertex shader.

    VertexLength - The length of VertexData.

    FragmentData - The contents of the vertex shader.

    FragmentLength - The length of FragmentData.

    ShaderType - The type of the shader.

Return Value:

    TRUE - The shader was successfully created.

    FALSE - The shader could not be created.

--*/
{
    PSHADER Shader;

    if ( !Name || !VertexData || !VertexLength || !FragmentData || !FragmentLength )
    {
        LogError("Cannot create invalid shader %s", Name);
        return FALSE;
    }

    LogInfo("Creating type %d shader %s", ShaderType, Name);

    Shader = PURPL_ALLOC(
        1,
        sizeof(SHADER) + VertexLength + FragmentLength
        );
    if ( !Shader )
    {
        LogError("Failed to allocate memory for shader %s", Name);
        return FALSE;
    }

    strncpy(
        Shader->Name,
        Name,
        PURPL_ARRAYSIZE(Shader->Name) -1
        );
    Shader->VertexData = Shader + 1;
    Shader->VertexLength = VertexLength;
    Shader->FragmentData = (PBYTE)(Shader + 1) + VertexLength;
    Shader->FragmentLength = FragmentLength;

    memmove(
        Shader->VertexData,
        VertexData,
        VertexLength
        );
    memmove(
        Shader->FragmentData,
        FragmentData,
        FragmentLength
        );

    Shader->Type = ShaderType;

    if ( RenderInterfaces[RenderApi].CreateShader )
    {
        LogTrace("Calling RenderInterfaces[%s].CreateShader(\"%s\")", RenderInterfaces[RenderApi].Name, Name);
        RenderInterfaces[RenderApi].CreateShader(Shader);
    }

    stbds_shput(
        Shaders,
        Name,
        Shader
        );

    return stbds_shget(
        Shaders,
        Name
        ) != NULL;
}

BOOLEAN
RenderLoadShader(
    _In_ PCSTR Name,
    _In_ SHADER_TYPE ShaderType
    )
/*++

Routine Description:

    Loads a shader and creates a shader object for it.

Arguments:

    Name - The name of the shader.

    ShaderType - The type of the shader.

Return Value:

    TRUE - The shader was successfully loaded.

    FALSE - The shader could not be loaded.

--*/
{
    BOOLEAN Succeeded;
    CHAR VertexPath[128];
    CHAR FragmentPath[128];
    PVOID VertexData;
    SIZE_T VertexLength;
    PVOID FragmentData;
    SIZE_T FragmentLength;

    if ( !Name )
    {
        LogDebug("Unable to load shader with no name");
        return FALSE;
    }

    switch ( RenderApi )
    {
    default:
    case RenderApiNone:
    {
        strncpy(
            VertexPath,
            Name,
            PURPL_ARRAYSIZE(VertexPath) - 1
            );
        strncpy(
            FragmentPath,
            Name,
            PURPL_ARRAYSIZE(FragmentPath) - 1
            );
        break;
    }
    case RenderApiDX12:
    {
        snprintf(
            VertexPath,
            PURPL_ARRAYSIZE(VertexPath),
            ASSETS_PREFIX "shaders/directx/%s.hlsl",
            Name
            );
        snprintf(
            FragmentPath,
            PURPL_ARRAYSIZE(FragmentPath),
            ASSETS_PREFIX "shaders/directx/%s.hlsl",
            Name
            );
        break;
    }
    case RenderApiVulkan:
    {
        snprintf(
            VertexPath,
            PURPL_ARRAYSIZE(VertexPath),
            ASSETS_PREFIX "shaders/vulkan/%s.vert.spv",
            Name
            );
        snprintf(
            FragmentPath,
            PURPL_ARRAYSIZE(FragmentPath),
            ASSETS_PREFIX "shaders/vulkan/%s.frag.spv",
            Name
            );
        break;
    }
    }

    LogInfo("Loading type %d shader %s", ShaderType, Name);

    VertexLength = 0;
    VertexData = FsReadFile(
        VertexPath,
        0,
        &VertexLength,
        0
        );
    if ( !VertexData )
    {
        LogError("Failed to read shader %s", VertexPath);
        return FALSE;
    }

    FragmentLength = 0;
    FragmentData = FsReadFile(
        FragmentPath,
        0,
        &FragmentLength,
        0
        );
    if ( !FragmentData )
    {
        LogError("Failed to read shader %s", FragmentPath);
        return FALSE;
    }

    Succeeded = RenderCreateShader(
        Name,
        VertexData,
        VertexLength,
        FragmentData,
        FragmentLength,
        ShaderType
        );
    PURPL_FREE(VertexData);
    PURPL_FREE(FragmentData);
    return Succeeded;
}

VOID
RenderDestroyShader(
    _In_opt_ PCSTR Name
    )
{
    PSHADER Shader;

    if ( !Name )
    {
        return;
    }

    Shader = stbds_shget(
        Shaders,
        Name
        );

    if ( RenderInterfaces[RenderApi].DestroyShader )
    {
        LogTrace("Calling RenderInterfaces[%s].DestroyShader(\"%s\")", RenderInterfaces[RenderApi].Name, Name);
        RenderInterfaces[RenderApi].DestroyShader(Shader);
    }

    PURPL_FREE(Shader);

    stbds_shdel(
        Shaders,
        Name
        );
}

PSHADER
RenderGetShader(
    _In_opt_ PCSTR Name
    )
{
    return stbds_shget(
        Shaders,
        Name
        );
}

BOOLEAN
RenderCreateModel(
    _In_ PCSTR Name,
    _In_ PCSTR Material,
    _In_reads_(VertexCount * sizeof(VERTEX)) PVERTEX Vertices,
    _In_ SIZE_T VertexCount,
    _In_reads_(IndexCount * sizeof(ivec3)) ivec3* Indices,
    _In_ SIZE_T IndexCount
    )
{
    PMESH Mesh;

    Mesh = CreateMesh(
        Material,
        Vertices,
        VertexCount,
        Indices,
        IndexCount
        );

    return RenderUseMesh(
        Name,
        Mesh
        );
}

BOOLEAN
RenderLoadModel(
    _In_ PCSTR Name,
    _In_ UINT32 Index
    )
{
    PMESH Mesh;
    PCSTR Path;
    BOOLEAN Succeeded;

    Path = CommonFormatString(ASSETS_PREFIX "models/%s.pmdl", Name);

    if ( RenderInterfaces[RenderApi].LoadModel )
    {
        stbds_shput(
            Models,
            Path,
            RenderInterfaces[RenderApi].LoadModel(Path)
            );
        Succeeded = stbds_shget(
            Models,
            Path
            ) != NULL;
    }
    else
    {
        Mesh = LoadMesh(Path);
        Succeeded = RenderUseMesh(
            Name,
            Mesh
            );
    }

    PURPL_FREE(Path);
    return Succeeded;
}

BOOLEAN
RenderUseMesh(
    _In_ PCSTR Name,
    _In_ PMESH Mesh
    )
{
    PMODEL Model;

    if ( !Name || !Mesh )
    {
        LogError("Not creating invalid model %s", Name);
        return FALSE;
    }

    Model = PURPL_ALLOC(
        1,
        sizeof(MODEL)
        );
    if ( !Model )
    {
        LogError("Failed to allocate memory for model %s", Name);
        return FALSE;
    }

    strncpy(
        Model->Name,
        Name,
        PURPL_ARRAYSIZE(Model->Name) - 1
        );
    Model->Mesh = Mesh;
    Model->Material = stbds_shget(
        Materials,
        Model->Mesh->Material
        );

    if ( RenderInterfaces[RenderApi].UseMesh )
    {
        LogTrace("Calling RenderInterfaces[%s].UseMesh(\"%s\")", RenderInterfaces[RenderApi].Name, Name);
        RenderInterfaces[RenderApi].UseMesh(Model);
    }

    stbds_shput(
        Models,
        Model->Name,
        Model
        );

    return stbds_shget(
        Models,
        Model->Name
        ) != NULL;
}

VOID
RenderDestroyModel(
    _In_opt_ PCSTR Name
    )
{
    PMODEL Model;

    if ( !Name )
    {
        return;
    }

    Model = stbds_shget(
        Models,
        Name
        );
    if ( !Model )
    {
        return;
    }
    stbds_shdel(
        Models,
        Name
        );

    if ( RenderInterfaces[RenderApi].DestroyModel )
    {
        LogTrace("Calling RenderInterfaces[%s].DestroyModel(\"%s\")", RenderInterfaces[RenderApi].Name, Name);
        RenderInterfaces[RenderApi].DestroyModel(Model);
    }

    if ( Model->Mesh->DataSeparate )
    {
        PURPL_FREE(Model->Mesh->Vertices);
        if ( Model->Mesh->IndexCount )
        {
            PURPL_FREE(Model->Mesh->Indices);
        }
    }
    PURPL_FREE(Model->Mesh);
    PURPL_FREE(Model);
}

PMODEL
RenderGetModel(
    _In_opt_ PCSTR Name
    )
{
    return stbds_shget(
        Models,
        Name
        );
}

BOOLEAN
RenderCreateTexture(
    _In_ PCSTR Name,
    _In_ UINT32 Width,
    _In_ UINT32 Height,
    _In_ TEXTURE_FORMAT Format,
    _In_ PVOID Data
    )
{
    PTEXTURE Texture;

    Texture = CreateTexture(
        Format,
        Width,
        Height,
        Data
        );

    return RenderUseTexture(
        Name,
        Texture
        );
}

BOOLEAN
RenderLoadTexture(
    _In_ PCSTR Name
    )
{
    PTEXTURE Texture;
    PCSTR Path;

    Path = CommonFormatTempString(ASSETS_PREFIX "textures/%s.ptex", Name);

    if ( RenderInterfaces[RenderApi].LoadTexture )
    {
        stbds_shput(
            Textures,
            Path,
            RenderInterfaces[RenderApi].LoadTexture(Path)
            );
        return stbds_shget(
            Textures,
            Path
            ) != NULL;
    }
    else
    {
        Texture = LoadTexture(Path);
        return RenderUseTexture(
            Name,
            Texture
            );
    }
}

BOOLEAN
RenderUseTexture(
    _In_ PCSTR Name,
    _In_ PTEXTURE Texture
    )
{
    PRENDER_TEXTURE RenderTexture;

    if ( !Name || !Texture )
    {
        LogError("Not creating invalid render texture %s", Name);
        return FALSE;
    }

    RenderTexture = PURPL_ALLOC(
        1,
        sizeof(RENDER_TEXTURE)
        );
    if ( !RenderTexture )
    {
        LogError("Failed to allocate memory for texture %s", Name);
        return FALSE;
    }

    strncpy(
        RenderTexture->Name,
        Name,
        PURPL_ARRAYSIZE(RenderTexture->Name) - 1
        );
    RenderTexture->Texture = Texture;

    if ( RenderInterfaces[RenderApi].UseTexture )
    {
        LogTrace("Calling RenderInterfaces[%s].UseTexture(\"%s\")", RenderInterfaces[RenderApi].Name, Name);
        RenderInterfaces[RenderApi].UseTexture(RenderTexture);
    }

    stbds_shput(
        Textures,
        RenderTexture->Name,
        RenderTexture
        );

    return stbds_shget(
        Textures,
        RenderTexture->Name
        ) != NULL;
}

VOID
RenderDestroyTexture(
    _In_opt_ PCSTR Name
    )
{
    PRENDER_TEXTURE Texture;

    if ( !Name )
    {
        return;
    }

    Texture = stbds_shget(
        Textures,
        Name
        );
    if ( !Texture )
    {
        return;
    }
    stbds_shdel(
        Textures,
        Name
        );

    if ( RenderInterfaces[RenderApi].DestroyTexture )
    {
        LogTrace("Calling RenderInterfaces[%s].DestroyTexture(\"%s\")", RenderInterfaces[RenderApi].Name, Name);
        RenderInterfaces[RenderApi].DestroyTexture(Texture);
    }

    if ( Texture->Texture->DataSeparate )
    {
        PURPL_FREE(Texture->Texture->Pixels);
    }
    PURPL_FREE(Texture->Texture);
    PURPL_FREE(Texture);
}

PRENDER_TEXTURE
RenderGetTexture(
    _In_opt_ PCSTR Name
    )
{
    return stbds_shget(
        Textures,
        Name
        );
}

VOID
RenderAddMaterial(
    _In_ PCSTR Name,
    _In_ PCSTR ShaderName,
    _In_ PCSTR TextureName
    )
{
    PMATERIAL Material;

    if ( !Name || !ShaderName || !TextureName )
    {
        LogError("Ignoring request to create material %s from shader %s and texture %s", Name, ShaderName, TextureName);
        return;
    }

    LogInfo("Creating material %s from shader %s and texture %s", Name, ShaderName, TextureName);

    Material = PURPL_ALLOC(
        1,
        sizeof(MATERIAL)
        );
    if ( !Material )
    {
        LogError("Failed to allocate material %s", Name);
        return;
    }

    strncpy(
        Material->Name,
        Name,
        PURPL_ARRAYSIZE(Material->Name) - 1
        );
    Material->Shader = stbds_shget(
        Shaders,
        ShaderName
        );
    Material->Texture = stbds_shget(
        Textures,
        TextureName
        );

    stbds_shput(
        Materials,
        Name,
        Material
        );
}

VOID
RenderDeleteMaterial(
    _In_opt_ PCSTR Name
    )
{
    stbds_shdel(
        Materials,
        Name
        );
}

PMATERIAL
RenderGetMaterial(
    _In_opt_ PCSTR Name
    )
{
    return stbds_shget(
        Materials,
        Name
        );
}

ecs_entity_t ecs_id(RENDERABLE);

ecs_entity_t ecs_id(RenderDrawEntity);

VOID
RenderImport(
    ecs_world_t* World
    )
{
    LogTrace("Importing ECS Render module");
    ECS_MODULE(
        World,
        Render
        );

    ECS_COMPONENT_DEFINE(
        World,
        RENDERABLE
        );

    ECS_SYSTEM_DEFINE(
        World,
        RenderDrawEntity,
        EcsPostFrame,
        TRANSFORM,
        RENDERABLE
        );
}

VOID
RenderDrawEntity(
    _In_ ecs_iter_t* Iterator
    )
{
    PTRANSFORM Transforms;
    PRENDERABLE Renderables;
    INT32 i;

    Transforms = ecs_field(
        Iterator,
        TRANSFORM,
        1
        );
    Renderables = ecs_field(
        Iterator,
        RENDERABLE,
        2
        );
    if ( !Transforms || !Renderables )
    {
        return;
    }

    for ( i = 0; i < Iterator->count; i++ )
    {
        if ( InFrame )
        {
            switch ( Renderables[i].Type )
            {
            case RenderableTypeModel:
            {
                RENDER_MODEL_UNIFORM_DATA UniformData = {0};

                MathCreateTransformMatrix(
                    &Transforms[i],
                    UniformData.ModelTransform
                    );
                if ( RenderInterfaces[RenderApi].DrawModel && Renderables[i].Model )
                {
                    RenderInterfaces[RenderApi].DrawModel(
                        Renderables[i].Model,
                        &UniformData
                        );
                }
                break;
            }
            }
        }
    }
}
