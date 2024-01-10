#include "render.h"

ecs_entity_t ecs_id(MODEL);

static RENDER_API Api;
static RENDER_BACKEND Backend;

#ifdef PURPL_DIRECTX
extern
VOID
D3d12InitializeBackend(
    _Out_ PRENDER_BACKEND Backend
    );
#else
VOID
D3d12InitializeBackend(
    _Out_ PRENDER_BACKEND Backend
    )
{
    LogError("Why are you intializing Direct3D 12 on this platform?");
}
#endif

#ifdef PURPL_VULKAN
extern
VOID
VkInitializeBackend(
    _Out_ PRENDER_BACKEND Backend
    );
#else
VOID
VkInitializeBackend(
    _Out_ PRENDER_BACKEND Backend
    )
{
    LogError("Why are you intializing Vulkan on this platform?");
}
#endif

VOID
RdrInitialize(
    _In_ ecs_iter_t* Iterator
    )
{
    LogInfo("Initializing renderer");

#ifdef PURPL_DIRECTX
    Api = RenderApiDirect3D12;
#elif defined(PURPL_VULKAN)
    Api = RenderApiVulkan;
#endif
    switch ( Api )
    {
    default:
    case RenderApiNone:
        break;
    case RenderApiDirect3D12:
        D3d12InitializeBackend(&Backend);
        break;
    case RenderApiVulkan:
        VkInitializeBackend(&Backend);
        break;
    }

    if ( Backend.Initialize  )
    {
        Backend.Initialize();
    }

    LogInfo("Renderer initialization succeeded");
}
ecs_entity_t ecs_id(RdrInitialize);

VOID
RdrBeginFrame(
    _In_ ecs_iter_t* Iterator
    )
{
    if ( Backend.BeginFrame )
    {
        Backend.BeginFrame();
    }
}
ecs_entity_t ecs_id(RdrBeginFrame);

VOID
RdrEndFrame(
    _In_ ecs_iter_t* Iterator
    )
{
    if ( Backend.EndFrame )
    {
        Backend.EndFrame();
    }
}
ecs_entity_t ecs_id(RdrEndFrame);

VOID
RdrShutdown(
    VOID
    )
{
    if ( Backend.Shutdown )
    {
        Backend.Shutdown();
    }
}

VOID
RenderImport(
    _In_ ecs_world_t* World
    )
{
    LogTrace("Importing Render ECS module");

    ECS_MODULE(
        World,
        Render
        );

    ECS_SYSTEM_DEFINE(
        World,
        RdrInitialize,
        EcsOnStart
        );
    ECS_SYSTEM_DEFINE(
        World,
        RdrBeginFrame,
        EcsPreUpdate
        );
    ECS_SYSTEM_DEFINE(
        World,
        RdrEndFrame,
        EcsPostUpdate
        );
}
