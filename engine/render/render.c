#include "render.h"

VOID
RdrInitialize(
    _In_ ecs_iter_t* Iterator
    )
{

}
ecs_entity_t ecs_id(RdrInitialize);

VOID
RdrBeginFrame(
    _In_ ecs_iter_t* Iterator
    )
{

}
ecs_entity_t ecs_id(RdrBeginFrame);

VOID
RdrEndFrame(
    _In_ ecs_iter_t* Iterator
    )
{

}
ecs_entity_t ecs_id(RdrEndFrame);

VOID
RdrShutdown(
    VOID
    )
{

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
