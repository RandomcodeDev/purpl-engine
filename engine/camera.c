/// @file camera.c
///
/// @brief This file implements camera functions.
///
/// @copyright (c) 2024 Randomcode Developers

#include "engine/render/render.h"

#include "camera.h"
#include "components.h"
#include "entity.h"

ecs_entity_t ecs_id(CAMERA);

VOID CamAddPerspective(_In_ ecs_entity_t Entity, _In_ DOUBLE FieldOfView, _In_ DOUBLE Aspect, _In_ DOUBLE NearClip,
                       _In_ DOUBLE FarClip)
{
    PCAMERA Camera = ecs_emplace(EcsGetWorld(), Entity, CAMERA);

    Camera->Perspective = TRUE;
    Camera->FieldOfView = glm_rad((FLOAT)FieldOfView);
    Camera->Aspect = Aspect;
    Camera->NearClip = NearClip;
    Camera->FarClip = FarClip;
}

VOID CamAddOrthographic(_In_ ecs_entity_t Entity)
{
    PCAMERA Camera = ecs_emplace(EcsGetWorld(), Entity, CAMERA);

    Camera->Perspective = FALSE;
}

VOID CamUpdate(_In_ ecs_iter_t *Iterator)
{
    PCAMERA Camera = ecs_field(Iterator, CAMERA, 1);
    PPOSITION Position = ecs_field(Iterator, POSITION, 2);

    for (UINT32 i = 0; i < Iterator->count; i++)
    {
        glm_lookat_lh(Position[i].Value, (vec3){0.0, 0.0, 0.0}, (vec3){0.0, 1.0, 0.0}, Camera[i].View);

        if (Camera[i].Perspective)
        {
            glm_perspective_lh_no((FLOAT)Camera[i].FieldOfView, (FLOAT)Camera[i].Aspect, (FLOAT)Camera[i].NearClip,
                                  (FLOAT)Camera[i].FarClip, Camera[i].Projection);
        }
        else
        {
            glm_ortho_lh_no(0.0, RdrGetWidth(), RdrGetHeight(), 0.0, (FLOAT)Camera[i].NearClip,
                            (FLOAT)Camera[i].FarClip, Camera[i].Projection);
        }
    }
}
ecs_entity_t ecs_id(CamUpdate);

VOID CameraImport(_In_ ecs_world_t *World)
{
    LogTrace("Importing Camera ECS module");

    ECS_MODULE(World, Camera);

    ECS_COMPONENT_DEFINE(World, CAMERA);

    ECS_SYSTEM(World, CamUpdate, EcsOnUpdate, CAMERA, POSITION);
}
