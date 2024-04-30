#include "components.h"

ecs_entity_t ecs_id(POSITION);
ecs_entity_t ecs_id(ROTATION);
ecs_entity_t ecs_id(SCALE);

VOID CoreImport(_In_ ecs_world_t *World)
{
    ECS_MODULE(World, Core);

    ECS_COMPONENT_DEFINE(World, POSITION);
    ECS_COMPONENT_DEFINE(World, ROTATION);
    ECS_COMPONENT_DEFINE(World, SCALE);
}
