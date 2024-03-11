#include "engine/engine.h"

#include "render.h"

ecs_entity_t ecs_id(MODEL);

static RENDER_BACKEND Backend;
static UINT32 LastWidth;
static UINT32 LastHeight;

#ifdef PURPL_DIRECTX
extern VOID Dx12InitializeBackend(_Out_ PRENDER_BACKEND Backend);
#else
VOID Dx12InitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    UNREFERENCED_PARAMETER(Backend);
    LogError("Why are you intializing DX12 on this platform?");
}
#endif

#ifdef PURPL_VULKAN
extern VOID VlkInitializeBackend(_Out_ PRENDER_BACKEND Backend);
#else
VOID VlkInitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    LogError("Why are you intializing Vulkan on this platform?");
}
#endif

VOID RdrDefineVariables(VOID)
{
    CONFIGVAR_DEFINE_FLOAT("rdr_scale", 1.0f, FALSE, ConfigVarSideClientOnly, FALSE);

#ifdef PURPL_GDKX
    static CONST RENDER_API DefaultApi = RenderApiDirect3D12;
#else
    static CONST RENDER_API DefaultApi = RenderApiVulkan;
#endif

    CONFIGVAR_DEFINE_INT("rdr_api", DefaultApi, TRUE, ConfigVarSideClientOnly, FALSE);

    CONFIGVAR_DEFINE_INT("rdr_clear_colour", 0x000000FF, FALSE, ConfigVarSideClientOnly, FALSE);
}

VOID RdrInitialize(_In_ ecs_iter_t *Iterator)
{
    UNREFERENCED_PARAMETER(Iterator);

    LogInfo("Initializing renderer");

    switch (CONFIGVAR_GET_INT("rdr_api"))
    {
    default:
    case RenderApiNone:
        break;
    case RenderApiDirect3D12:
        Dx12InitializeBackend(&Backend);
        break;
    case RenderApiVulkan:
        VlkInitializeBackend(&Backend);
        break;
    }

    if (Backend.Initialize)
    {
        Backend.Initialize();
    }

    LogInfo("Renderer initialization succeeded");
}
ecs_entity_t ecs_id(RdrInitialize);

VOID RdrBeginFrame(_In_ ecs_iter_t *Iterator)
{
    UNREFERENCED_PARAMETER(Iterator);

    if (Backend.BeginFrame)
    {
        Backend.BeginFrame(EngHasVideoResized() || RdrGetWidth() != LastWidth || RdrGetHeight() != LastHeight);
    }
}
ecs_entity_t ecs_id(RdrBeginFrame);

VOID RdrDrawModel(_In_ ecs_iter_t *Iterator)
{
    PMODEL Model = ecs_field(Iterator, MODEL, 1);
    PTRANSFORM Transform = ecs_field(Iterator, TRANSFORM, 2);
    CONST PCAMERA Camera = ecs_get(EcsGetWorld(), EngGetMainCamera(), CAMERA);
    CalculateCameraMatrices(Camera);

    if (Backend.DrawModel)
    {
        for (INT32 i = 0; i < Iterator->count; i++)
        {
            mat4 ModelTransform;
            MthCreateTransformMatrix(&Transform[i], ModelTransform);
            Backend.DrawModel(&Model[i], ModelTransform, Camera->View, Camera->Projection);
        }
    }
}
ecs_entity_t ecs_id(RdrDrawModel);

VOID RdrEndFrame(_In_ ecs_iter_t *Iterator)
{
    UNREFERENCED_PARAMETER(Iterator);

    if (Backend.EndFrame)
    {
        Backend.EndFrame();
    }

    LastWidth = RdrGetWidth();
    LastHeight = RdrGetHeight();
}
ecs_entity_t ecs_id(RdrEndFrame);

VOID RdrShutdown(VOID)
{
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
    ECS_SYSTEM_DEFINE(World, RdrDrawModel, EcsOnUpdate, MODEL, TRANSFORM);
    ECS_SYSTEM_DEFINE(World, RdrEndFrame, EcsPostUpdate);
}

PVOID RdrUseTexture(_In_ PTEXTURE Texture)
{
    if (Backend.UseTexture)
    {
        return Backend.UseTexture(Texture);
    }
    else
    {
        return Texture;
    }
}

VOID RdrReleaseTexture(_In_ PVOID TextureHandle)
{
    if (Backend.ReleaseTexture)
    {
        Backend.ReleaseTexture(TextureHandle);
    }
}

BOOLEAN RdrCreateMaterial(_Out_ PMATERIAL Material, _In_ PVOID TextureHandle, _In_z_ PCSTR ShaderName)
{
    if (!Material || !TextureHandle || !ShaderName)
    {
        return FALSE;
    }

    Material->TextureHandle = TextureHandle;
    Material->ShaderName = ShaderName;

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

BOOLEAN RdrCreateModel(_Out_ PMODEL Model, _In_ PMESH Mesh, _In_ PMATERIAL Material)
{
    if (!Model || !Mesh || !Material)
    {
        return FALSE;
    }

    Model->Material = Material;
    if (Backend.CreateModel)
    {
        Backend.CreateModel(Model, Mesh);
    }
    else
    {
        Model->MeshHandle = Mesh;
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
