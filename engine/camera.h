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
    DOUBLE Aspect;

    mat4 View;
    mat4 Projection;
})

/// @brief Add a perspective camera to the given entity
///
/// @param[in] Entity      The entity to add the camera to.
/// @param[in] FieldOfView The camera's FOV.
/// @param[in] Aspect      The aspect ratio of the camera.
/// @param[in] NearClip    The near clipping distance.
/// @param[in] FarClip     The far clipping distance.
extern VOID CamAddPerspective(_In_ ecs_entity_t Entity, _In_ DOUBLE FieldOfView, _In_ DOUBLE Aspect,
                              _In_ DOUBLE NearClip, _In_ DOUBLE FarClip);

/// @brief Add a perspective camera to the given entity
///
/// @param[in] Entity      The entity to add the camera to.
/// @param[out] Camera   The camera to initialize.
extern VOID CamAddOrthographic(_In_ ecs_entity_t Entity);

extern ECS_SYSTEM_DECLARE(CamUpdate);
