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

VOID CamAddPerspective(_In_ ecs_entity_t Entity, _In_ DOUBLE FieldOfView, _In_ BOOLEAN FixedFov, _In_ DOUBLE NearClip,
                       _In_ DOUBLE FarClip)
{
    PCAMERA Camera = ecs_emplace(EcsGetWorld(), Entity, CAMERA);

    Camera->Perspective = TRUE;
    Camera->FieldOfView = glm_rad((FLOAT)FieldOfView);
    Camera->FixedFov = FixedFov;
    Camera->NearClip = NearClip;
    Camera->FarClip = FarClip;
}

VOID CamAddOrthographic(_In_ ecs_entity_t Entity)
{
    PCAMERA Camera = ecs_emplace(EcsGetWorld(), Entity, CAMERA);

    Camera->Perspective = FALSE;
}

// These are to reduce branches
PURPL_MAKE_TAG(struct, CAMERA_FUNCS, {
    void (*LookAt)(vec3, vec3, vec3, mat4);
    void (*Perspective)(FLOAT, FLOAT, FLOAT, FLOAT, mat4);
    void (*Orthographic)(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, mat4);
    FLOAT UpSign;
});
static CONST CAMERA_FUNCS CameraFuncs[RenderApiCount] = {
    // Vulkan
    {glm_lookat_lh, glm_perspective_lh_no, glm_ortho_lh_no, 1.0},
    // DX 12
    {glm_lookat_lh, glm_perspective_lh_no, glm_ortho_lh_no, 1.0},
    // DX 9
    {glm_lookat_lh, glm_perspective_lh_no, glm_ortho_lh_no, 1.0},
    // OpenGL
    {glm_lookat_rh, glm_perspective_rh_zo, glm_ortho_rh_zo, -1.0},
};
static PCCAMERA_FUNCS CurrentCameraFuncs;

VOID CamDefineVariables(VOID)
{
    CONFIGVAR_DEFINE_FLOAT("cam_fov", 78.0, FALSE, ConfigVarSideClientOnly, FALSE, FALSE);
}

VOID CamGetVectors(_In_ ecs_entity_t Camera, _Out_opt_ vec3 Forward, _Out_opt_ vec3 Up)
{
    mat4 RotationMatrix = {0};
    PCROTATION Rotation = ecs_get(EcsGetWorld(), Camera, ROTATION);
    if (Rotation)
    {
        glm_euler_zyx(Rotation->Value, RotationMatrix);
    }
    else
    {
        glm_mat4_identity(RotationMatrix);
    }

    if (Forward)
    {
        glm_vec3_copy((vec3){0.0, 0.0, 1.0}, Forward);
        glm_mat4_mulv3(RotationMatrix, Forward, 1.0, Forward);
    }

    if (Up)
    {
        glm_vec3_copy((vec3){0.0, CurrentCameraFuncs->UpSign, 0.0}, Up);
        glm_mat4_mulv3(RotationMatrix, Up, 1.0, Up);
    }
}

VOID CamUpdate(_In_ ecs_iter_t *Iterator)
{
    PCAMERA Camera = ecs_field(Iterator, CAMERA, 1);
    PCPOSITION Position = ecs_field(Iterator, POSITION, 2);

    for (INT32 i = 0; i < Iterator->count; i++)
    {
        if (!Camera[i].FixedFov)
        {
            Camera[i].FieldOfView = glm_rad(CONFIGVAR_GET_FLOAT("cam_fov"));
        }

        vec3 Forward = {0};
        vec3 Up = {0};
        CamGetVectors(Iterator->entities[i], Forward, Up);

        vec3 Center = {0};
        glm_vec3_add(Position[i].Value, Forward, Center);
        CurrentCameraFuncs->LookAt(Position[i].Value, Center, Up, Camera[i].View);

        if (Camera[i].Perspective)
        {
            CurrentCameraFuncs->Perspective((FLOAT)Camera[i].FieldOfView,
                                            (FLOAT)((DOUBLE)RdrGetWidth() / (DOUBLE)RdrGetHeight()),
                                            (FLOAT)Camera[i].NearClip, (FLOAT)Camera[i].FarClip, Camera[i].Projection);
        }
        else
        {
            CurrentCameraFuncs->Orthographic(0.0, RdrGetWidth(), RdrGetHeight(), 0.0, (FLOAT)Camera[i].NearClip,
                                             (FLOAT)Camera[i].FarClip, Camera[i].Projection);
        }
    }
}
ecs_entity_t ecs_id(CamUpdate);

VOID CameraImport(_In_ ecs_world_t *World)
{
    LogTrace("Importing Camera ECS module");

    CurrentCameraFuncs = &CameraFuncs[CONFIGVAR_GET_INT("rdr_api")];

    ECS_MODULE(World, Camera);

    ECS_COMPONENT_DEFINE(World, CAMERA);

    ECS_SYSTEM_DEFINE(World, CamUpdate, EcsOnUpdate, CAMERA, POSITION);
}
