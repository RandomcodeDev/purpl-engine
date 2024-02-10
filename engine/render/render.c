#include "engine/engine.h"

#include "render.h"

ecs_entity_t ecs_id(MODEL);

RENDER_API RenderApi;

static RENDER_BACKEND Backend;
static FLOAT Scale;
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

VOID RdrInitialize(_In_ ecs_iter_t *Iterator)
{
    LogInfo("Initializing renderer");
    RdrSetScale(1.0f);

//#ifdef PURPL_DIRECTX
//    RenderApi = RenderApiDirect3D12;
#if defined(PURPL_VULKAN)
    RenderApi = RenderApiVulkan;
#endif
    switch (RenderApi)
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
    if (Backend.BeginFrame)
    {
        Backend.BeginFrame(EngHasVideoResized() || RdrGetWidth() != LastWidth || RdrGetHeight() != LastHeight);
    }
}
ecs_entity_t ecs_id(RdrBeginFrame);

VOID RdrEndFrame(_In_ ecs_iter_t *Iterator)
{
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

    ECS_SYSTEM_DEFINE(World, RdrInitialize, EcsOnStart);
    ECS_SYSTEM_DEFINE(World, RdrBeginFrame, EcsPreUpdate);
    ECS_SYSTEM_DEFINE(World, RdrEndFrame, EcsPostUpdate);
}

FLOAT
RdrGetScale(VOID)
{
    return Scale;
}

FLOAT
RdrSetScale(FLOAT NewScale)
{
    FLOAT OldScale = Scale;
    Scale = NewScale;
    return OldScale;
}

UINT32
RdrGetWidth(VOID)
{
    UINT32 Width;

    VidGetSize(&Width, NULL);

    return Width * Scale;
}

UINT32
RdrGetHeight(VOID)
{
    UINT32 Height;

    VidGetSize(NULL, &Height);

    return Height * Scale;
}
