/// @file camera.h
/// 
/// @brief This file defines structures and functions for cameras.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "platform/video.h"

#include "mathutil.h"

/// @brief A camera
typedef struct CAMERA
{
    vec3 Position;
    vec4 Rotation;

    DOUBLE NearClip;
    DOUBLE FarClip;
    BOOLEAN Perspective;

    DOUBLE FieldOfView;
    DOUBLE Aspect;

    mat4 View;
    mat4 Projection;
    BOOLEAN Changed; // Set to TRUE to allow the above to be recalculated
} CAMERA, *PCAMERA;
extern ECS_COMPONENT_DECLARE(CAMERA);

/// @brief Initializes a perspective camera.
///
/// @param[in] Position    The camera's position.
/// @param[in] Rotation    The camera's rotation.
/// @param[in] FieldOfView The camera's FOV.
/// @param[in] Aspect      The aspect ratio of the camera.
/// @param[in] NearClip    The near clipping distance.
/// @param[in] FarClip     The far clipping distance.
/// @param[out] Camera      The camera to initialize.
extern VOID InitializePerspectiveCamera(_In_ vec3 Position, _In_ vec4 Rotation,
                                        _In_ DOUBLE FieldOfView,
                                        _In_ DOUBLE Aspect,
                                        _In_ DOUBLE NearClip,
                                        _In_ DOUBLE FarClip,
                                        _Out_ PCAMERA Camera);

/// @brief Initializes an orthographic camera.
///
/// @param[in] Position The position of the camera.
/// @param[in] Rotation The rotation of the camera.
/// @param[out] Camera   The camera to initialize.
extern VOID InitializeOrthographicCamera(_In_ vec3 Position, _In_ vec4 Rotation,
                                         _Out_ PCAMERA Camera);

/// @brief Updates a camera's matrices if necessary.
///
/// @param[in,out] Camera The camera to update.
extern VOID CalculateCameraMatrices(_Inout_ PCAMERA Camera);
