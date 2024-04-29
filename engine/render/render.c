#include "engine/engine.h"

#include "render.h"

ecs_entity_t ecs_id(MODEL);

static RENDER_BACKEND Backend;
static UINT32 LastWidth;
static UINT32 LastHeight;

#ifdef PURPL_DIRECTX
extern VOID Dx12InitializeBackend(_Out_ PRENDER_BACKEND Backend);
#else
static VOID Dx12InitializeBackend(_Out_ PRENDER_BACKEND Unused)
{
    UNREFERENCED_PARAMETER(Unused);
    LogError("Why are you initialising DX12 on this platform?");
}
#endif

#ifdef PURPL_VULKAN
extern VOID VlkInitializeBackend(_Out_ PRENDER_BACKEND Backend);
#else
static VOID VlkInitializeBackend(_Out_ PRENDER_BACKEND Unused)
{
    UNREFERENCED_PARAMETER(Unused);
    LogError("Why are you initialising Vulkan on this platform?");
}
#endif

#ifdef PURPL_OPENGL
extern VOID GlInitializeBackend(_Out_ PRENDER_BACKEND Backend);
#else
static VOID GlInitializeBackend(_Out_ PRENDER_BACKEND Unused)
{
    UNREFERENCED_PARAMETER(Unused);
    LogError("Why are you initialising OpenGL on this platform?");
}
#endif

VOID RdrDefineVariables(VOID)
{
    CONFIGVAR_DEFINE_FLOAT("rdr_scale", 1.0f, FALSE, ConfigVarSideClientOnly, FALSE, FALSE);

#ifdef PURPL_GDKX
    static CONST RENDER_API DefaultApi = RenderApiDirect3D12;
#elif defined PURPL_LEGACY_GRAPHICS
#ifdef PURPL_WIN32
    static CONST RENDER_API DefaultApi = RenderApiDirect3D9;
#else
    static CONST RENDER_API DefaultApi = RenderApiOpenGL;
#endif
#else
    static CONST RENDER_API DefaultApi = RenderApiVulkan;
#endif

    CONFIGVAR_DEFINE_INT("rdr_api", DefaultApi, TRUE, ConfigVarSideClientOnly, FALSE, FALSE);
    CONFIGVAR_DEFINE_BOOLEAN("rdr_lefthanded", DefaultApi == RenderApiDirect3D12 || DefaultApi == RenderApiDirect3D9,
                             FALSE, ConfigVarSideClientOnly, FALSE, TRUE);

    CONFIGVAR_DEFINE_INT("rdr_clear_colour", 0x000000FF, FALSE, ConfigVarSideClientOnly, FALSE, TRUE);
}

PURPL_MAKE_STRING_HASHMAP_ENTRY(SHADERMAP, RENDER_HANDLE);
PSHADERMAP RdrShaders;

static VOID LoadShaders(VOID)
{
    LogInfo("Loading shaders");

    if (Backend.LoadShader)
    {
        stbds_shput(RdrShaders, "main", Backend.LoadShader("main"));
    }
}

VOID RdrInitialize(_In_ ecs_iter_t *Iterator)
{
    UNREFERENCED_PARAMETER(Iterator);

    LogInfo("Initializing renderer using API %s", RdrGetApiName(CONFIGVAR_GET_INT("rdr_api")));

    CONFIGVAR_SET_BOOLEAN("rdr_lefthanded", CONFIGVAR_GET_INT("rdr_api") == RenderApiDirect3D12 ||
                                                CONFIGVAR_GET_INT("rdr_api") == RenderApiDirect3D9);

    switch (CONFIGVAR_GET_INT("rdr_api"))
    {
    case RenderApiDirect3D12:
        Dx12InitializeBackend(&Backend);
        break;
    case RenderApiVulkan:
        VlkInitializeBackend(&Backend);
        break;
    case RenderApiOpenGL:
        GlInitializeBackend(&Backend);
        break;
    default:
        CmnError("Unknown renderer");
    }

    if (Backend.Initialize)
    {
        Backend.Initialize();
    }

    LoadShaders();

    LogInfo("Renderer initialization succeeded");
}
ecs_entity_t ecs_id(RdrInitialize);

VOID RdrBeginFrame(_In_ ecs_iter_t *Iterator)
{
    if (CONFIGVAR_GET_BOOLEAN("ecs_in_init"))
    {
        return;
    }

    UNREFERENCED_PARAMETER(Iterator);

    BOOLEAN Resized = EngHasVideoResized() || RdrGetWidth() != LastWidth || RdrGetHeight() != LastHeight;
    CONST PCAMERA Camera = ecs_get(EcsGetWorld(), EngGetMainCamera(), CAMERA);
    Camera->Aspect = (DOUBLE)RdrGetWidth() / (DOUBLE)RdrGetHeight();
    Camera->Changed = Camera->Changed || Resized;
    EngUpdateCamera(Camera);
    RENDER_SCENE_UNIFORM Uniform = {0};
    glm_vec3_copy(Camera->Position, Uniform.CameraPosition);
    glm_mat4_copy(Camera->View, Uniform.View);
    glm_mat4_copy(Camera->Projection, Uniform.Projection);

    if (Backend.BeginFrame)
    {
        Backend.BeginFrame(Resized, &Uniform);
    }
}
ecs_entity_t ecs_id(RdrBeginFrame);

VOID RdrDrawModel(_In_ ecs_iter_t *Iterator)
{
    if (CONFIGVAR_GET_BOOLEAN("ecs_in_init"))
    {
        return;
    }

    PMODEL Model = ecs_field(Iterator, MODEL, 1);
//    PPOSITION Transform = ecs_field(Iterator, TRANSFORM, 2);

    if (Backend.DrawModel)
    {
        for (INT32 i = 0; i < Iterator->count; i++)
        {
            RENDER_OBJECT_UNIFORM Uniform = {0};
//            MthCreateTransformMatrix(&Transform[i], Uniform.Model);
            Backend.DrawModel(&Model[i], &Uniform);
        }
    }
}
ecs_entity_t ecs_id(RdrDrawModel);

VOID RdrEndFrame(_In_ ecs_iter_t *Iterator)
{
    if (CONFIGVAR_GET_BOOLEAN("ecs_in_init"))
    {
        return;
    }

    UNREFERENCED_PARAMETER(Iterator);

    if (Backend.EndFrame)
    {
        Backend.EndFrame();
    }

    LastWidth = RdrGetWidth();
    LastHeight = RdrGetHeight();
}
ecs_entity_t ecs_id(RdrEndFrame);

static VOID DestroyShaders(VOID)
{
    LogInfo("Destroying shaders");

    if (Backend.DestroyShader)
    {
        for (SIZE_T i = 0; i < stbds_shlenu(RdrShaders); i++)
        {
            PSHADERMAP Pair = &RdrShaders[i];
            LogDebug("Destroying shader %s", Pair->key);
            Backend.DestroyShader(Pair->value);
        }
    }
}

VOID RdrShutdown(VOID)
{
    DestroyShaders();

    if (Backend.Shutdown)
    {
        Backend.Shutdown();
    }
}

VOID RenderImport(_In_ ecs_world_t *World)
{
    LogTrace("Importing Render ECS module");

    ECS_MODULE(World, Render);

    ECS_COMPONENT_DEFINE(World, MODEL);

    ECS_SYSTEM_DEFINE(World, RdrInitialize, EcsOnStart);
    ECS_SYSTEM_DEFINE(World, RdrBeginFrame, EcsPreUpdate);
//    ECS_SYSTEM_DEFINE(World, RdrDrawModel, EcsOnUpdate, MODEL, TRANSFORM);
    ECS_SYSTEM_DEFINE(World, RdrEndFrame, EcsPostUpdate);
}

RENDER_HANDLE RdrLoadTexture(_In_z_ PCSTR Name)
{
    PTEXTURE Texture = LoadTexture(EngGetAssetPath(EngAssetDirectoryTextures, Name));

    if (Texture)
    {
        if (Backend.UseTexture)
        {
            RENDER_HANDLE Handle = Backend.UseTexture(Texture, Name);
            CmnFree(Texture); // Not needed anymore, should be uploaded to the GPU
            return Handle;
        }
        else
        {
            return (RENDER_HANDLE)Texture;
        }
    }

    return (RENDER_HANDLE)0;
}

VOID RdrDestroyTexture(_In_ RENDER_HANDLE TextureHandle)
{
    if (Backend.ReleaseTexture)
    {
        Backend.ReleaseTexture(TextureHandle);
    }
    else
    {
        CmnFree(TextureHandle);
    }
}

BOOLEAN RdrCreateMaterial(_Out_ PMATERIAL Material, _In_ RENDER_HANDLE TextureHandle, _In_z_ PCSTR ShaderName)
{
    RENDER_HANDLE ShaderHandle = ShaderName ? stbds_shget(RdrShaders, ShaderName) : 0;
    if (!Material || !TextureHandle || !ShaderName || !ShaderHandle)
    {
        if (Material)
        {
            memset(Material, 0, sizeof(MATERIAL));
        }
        return FALSE;
    }

    Material->TextureHandle = TextureHandle;
    Material->ShaderHandle = ShaderHandle;

    if (Backend.CreateMaterial)
    {
        Backend.CreateMaterial(Material);
    }

    return TRUE;
}

VOID RdrDestroyMaterial(_In_ PMATERIAL Material)
{
    if (Backend.DestroyMaterial)
    {
        Backend.DestroyMaterial(Material);
    }
}

BOOLEAN RdrLoadModel(_Out_ PMODEL Model, _In_z_ PCSTR Name, _In_ PMATERIAL Material)
{
    if (!Model || !Name || !Material)
    {
        return FALSE;
    }

    PMESH Mesh = LoadMesh(EngGetAssetPath(EngAssetDirectoryModels, Name));

    Model->Material = Material;
    if (Backend.CreateModel)
    {
        Backend.CreateModel(Model, Mesh, Name);
    }
    else
    {
        Model->MeshHandle = (RENDER_HANDLE)Mesh;
    }

    return TRUE;
}

VOID RdrDestroyModel(_In_ PMODEL Model)
{
    if (Backend.DestroyModel)
    {
        Backend.DestroyModel(Model);
    }
}

UINT32 RdrGetWidth(VOID)
{
    UINT32 Width;

    VidGetSize(&Width, NULL);

    return (UINT32)(Width * CONFIGVAR_GET_FLOAT("rdr_scale"));
}

UINT32 RdrGetHeight(VOID)
{
    UINT32 Height;

    VidGetSize(NULL, &Height);

    return (UINT32)(Height * CONFIGVAR_GET_FLOAT("rdr_scale"));
}

PCSTR RdrGetApiName(_In_ RENDER_API Api)
{
    static CONST PCSTR Names[] = {"Vulkan", "DirectX 12", "DirectX 9", "OpenGL", "Unknown"};

    if ((UINT32)Api < PURPL_ARRAYSIZE(Names))
    {
        return Names[Api];
    }
    else
    {
        return Names[PURPL_ARRAYSIZE(Names) - 1];
    }
}

PCSTR RdrGetGpuName(VOID)
{
    if (Backend.GetGpuName)
    {
        return Backend.GetGpuName();
    }
    else
    {
        return "Unknown";
    }
}
