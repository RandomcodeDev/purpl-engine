/// @file camera.h
///
/// @brief This file defines structures and functions for cameras.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "platform/video.h"

/// @brief A camera
PURPL_MAKE_COMPONENT(struct, CAMERA, {
    DOUBLE NearClip;
    DOUBLE FarClip;
    BOOLEAN Perspective;

    DOUBLE FieldOfView;
    BOOLEAN FixedFov;

    mat4 View;
    mat4 Projection;
})

/// @brief Define configuration variables
extern VOID CamDefineVariables(VOID);

/// @brief Add a perspective camera to the given entity
///
/// @param[in] Entity      The entity to add the camera to.
/// @param[in] FieldOfView The camera's FOV.
/// @param[in] FixedFov    Whether the camera's FOV is tied to cam_fov or not
/// @param[in] NearClip    The near clipping distance.
/// @param[in] FarClip     The far clipping distance.
extern VOID CamAddPerspective(_In_ ecs_entity_t Entity, _In_ DOUBLE FieldOfView, _In_ BOOLEAN FixedFov,
                              _In_ DOUBLE NearClip, _In_ DOUBLE FarClip);

/// @brief Add a perspective camera to the given entity
///
/// @param[in] Entity The entity to add the camera to.
/// @param[out] Camera The camera to initialize.
extern VOID CamAddOrthographic(_In_ ecs_entity_t Entity);

/// @brief Get the up and forward vectors of a camera
extern VOID CamGetVectors(_In_ ecs_entity_t Camera, _Out_opt_ vec3 Forward, _Out_opt_ vec3 Up);

extern ECS_SYSTEM_DECLARE(CamUpdate);
