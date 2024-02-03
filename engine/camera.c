/// @file camera.c
///
/// @brief This module implements camera functions.
///
/// @copyright (c) 2024 Randomcode Developers

#include "engine/render/render.h"

#include "camera.h"

ecs_entity_t ecs_id(CAMERA);

VOID CameraImport(_In_ ecs_world_t *World)
{
    LogTrace("Importing Camera ECS module");

    ECS_MODULE(World, Camera);

    ECS_COMPONENT_DEFINE(World, CAMERA);
}

VOID InitializePerspectiveCamera(_In_ vec3 Position, _In_ vec4 Rotation,
                                 _In_ DOUBLE FieldOfView, _In_ DOUBLE Aspect,
                                 _In_ DOUBLE NearClip, _In_ DOUBLE FarClip,
                                 _Out_ PCAMERA Camera)
{
    LogTrace("Initializing perspective camera");

    if (!Camera || !Position || !Rotation)
    {
        return;
    }

    memset(Camera, 0, sizeof(CAMERA));
    glm_vec3_copy(Position, Camera->Position);
    glm_vec3_copy(Rotation, Camera->Rotation);
    Camera->Perspective = TRUE;
    Camera->FieldOfView = glm_rad((FLOAT)FieldOfView);
    Camera->Aspect = Aspect;
    Camera->NearClip = NearClip;
    Camera->FarClip = FarClip;
    Camera->Changed = TRUE;
    CalculateCameraMatrices(Camera);
}

VOID InitializeOrthographicCamera(_In_ vec3 Position, _In_ vec4 Rotation,
                                  _Out_ PCAMERA Camera)
{
    LogTrace("Initializing orthographic camera");

    if (!Camera || !Position || !Rotation)
    {
        return;
    }

    memset(Camera, 0, sizeof(CAMERA));
    glm_vec3_copy(Position, Camera->Position);
    glm_vec3_copy(Rotation, Camera->Rotation);
    Camera->Perspective = FALSE;
    CalculateCameraMatrices(Camera);
}

VOID CalculateCameraMatrices(_Inout_ PCAMERA Camera)
{
    UINT32 Width;
    UINT32 Height;

    if (!Camera || !Camera->Changed)
    {
        return;
    }

    if (Camera->Perspective)
    {
        if (RenderApi == RenderApiDirect3D12)
        {
            glm_lookat_lh(Camera->Position, (vec3){0.0, 0.0, 0.0},
                          (vec3){0.0, 1.0, 0.0}, Camera->View);
            glm_perspective_lh_no((FLOAT)Camera->FieldOfView,
                                  (FLOAT)Camera->Aspect,
                                  (FLOAT)Camera->NearClip,
                                  (FLOAT)Camera->FarClip, Camera->Projection);
        }
        else
        {
            glm_lookat_rh(Camera->Position, (vec3){0.0, 0.0, 0.0},
                          (vec3){0.0, -1.0, 0.0}, Camera->View);
            glm_perspective_rh_no((FLOAT)Camera->FieldOfView,
                                  (FLOAT)Camera->Aspect,
                                  (FLOAT)Camera->NearClip,
                                  (FLOAT)Camera->FarClip, Camera->Projection);
        }
    }
    else
    {
        VidGetSize(&Width, &Height);

        if (RenderApi == RenderApiDirect3D12)
        {
            glm_lookat_lh(Camera->Position, (vec3){0.0, 0.0, 0.0},
                          (vec3){0.0, 1.0, 0.0}, Camera->View);
            glm_ortho_lh_no(0.0, (FLOAT)Width, (FLOAT)Height, 0.0,
                            (FLOAT)Camera->NearClip, (FLOAT)Camera->FarClip,
                            Camera->Projection);
        }
        else
        {
            glm_lookat_rh(Camera->Position, (vec3){0.0, 0.0, 0.0},
                          (vec3){0.0, -1.0, 0.0}, Camera->View);
            glm_ortho_rh_no(0.0, (FLOAT)Width, (FLOAT)Height, 0.0,
                            (FLOAT)Camera->NearClip, (FLOAT)Camera->FarClip,
                            Camera->Projection);
        }
    }

    Camera->Changed = FALSE;
}
