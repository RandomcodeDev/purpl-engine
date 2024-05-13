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

VOID CamAddPerspective(_In_ ecs_entity_t Entity, _In_ DOUBLE FieldOfView, _In_ BOOLEAN FixedFov, _In_ DOUBLE Aspect,
                       _In_ DOUBLE NearClip, _In_ DOUBLE FarClip)
{
    PCAMERA Camera = ecs_emplace(EcsGetWorld(), Entity, CAMERA);

    Camera->Perspective = TRUE;
    Camera->FieldOfView = glm_rad((FLOAT)FieldOfView);
    Camera->FixedFov = FixedFov;
    Camera->Aspect = Aspect;
    Camera->NearClip = NearClip;
    Camera->FarClip = FarClip;
}

VOID CamAddOrthographic(_In_ ecs_entity_t Entity)
{
    PCAMERA Camera = ecs_emplace(EcsGetWorld(), Entity, CAMERA);

    Camera->Perspective = FALSE;
}

// These are to reduce branches

static void (*LookAt)(vec3, vec3, vec3, mat4);
static void (*Perspective)(FLOAT, FLOAT, FLOAT, FLOAT, mat4);
static void (*Orthographic)(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, mat4);
static FLOAT Up;

VOID CamDefineVariables(VOID)
{
    CONFIGVAR_DEFINE_FLOAT("cam_fov", 78.0, FALSE, ConfigVarSideClientOnly, FALSE, FALSE);
}

VOID CamUpdate(_In_ ecs_iter_t *Iterator)
{
    PCAMERA Camera = ecs_field(Iterator, CAMERA, 1);
    PPOSITION Position = ecs_field(Iterator, POSITION, 2);

    for (INT32 i = 0; i < Iterator->count; i++)
    {
        if (!Camera[i].FixedFov)
        {
            Camera[i].FieldOfView = glm_rad(CONFIGVAR_GET_FLOAT("cam_fov"));
        }

        LookAt(Position[i].Value, (vec3){0.0, 0.0, 0.0}, (vec3){0.0, Up, 0.0}, Camera[i].View);

        if (Camera[i].Perspective)
        {
            Camera[i].Aspect = (DOUBLE)RdrGetWidth() / (DOUBLE)RdrGetHeight();
            Perspective((FLOAT)Camera[i].FieldOfView, (FLOAT)Camera[i].Aspect, (FLOAT)Camera[i].NearClip,
                        (FLOAT)Camera[i].FarClip, Camera[i].Projection);
        }
        else
        {
            Orthographic(0.0, RdrGetWidth(), RdrGetHeight(), 0.0, (FLOAT)Camera[i].NearClip, (FLOAT)Camera[i].FarClip,
                         Camera[i].Projection);
        }
    }
}
ecs_entity_t ecs_id(CamUpdate);

VOID CameraImport(_In_ ecs_world_t *World)
{
    LogTrace("Importing Camera ECS module");

    if (CONFIGVAR_GET_INT("rdr_api") == RenderApiOpenGL)
    {
        LookAt = glm_lookat_rh;
        Perspective = glm_perspective_rh_no;
        Orthographic = glm_ortho_rh_no;
        Up = -1.0;
    }
    else
    {
        LookAt = glm_lookat_lh;
        Perspective = glm_perspective_lh_no;
        Orthographic = glm_ortho_lh_no;
        Up = 1.0;
    }

    ECS_MODULE(World, Camera);

    ECS_COMPONENT_DEFINE(World, CAMERA);

    ECS_SYSTEM_DEFINE(World, CamUpdate, EcsOnUpdate, CAMERA, POSITION);
}
