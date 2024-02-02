/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    mathutil.h

Abstract:

    This module defines structures and functions for cameras.

--*/

#pragma once

#include "purpl/purpl.h"

#include "platform/video.h"

#include "mathutil.h"

//
// Camera
//

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
/// @param Position    The camera's position.
/// @param Rotation    The camera's rotation.
/// @param FieldOfView The camera's FOV.
/// @param Aspect      The aspect ratio of the camera.
/// @param NearClip    The near clipping distance.
/// @param FarClip     The far clipping distance.
/// @param Camera      The camera to initialize.
extern VOID InitializePerspectiveCamera(_In_ vec3 Position, _In_ vec4 Rotation,
                                        _In_ DOUBLE FieldOfView,
                                        _In_ DOUBLE Aspect,
                                        _In_ DOUBLE NearClip,
                                        _In_ DOUBLE FarClip,
                                        _Out_ PCAMERA Camera);

/// @brief Initializes an orthographic camera.
///
/// @param Position The position of the camera.
/// @param Rotation The rotation of the camera.
/// @param Camera   The camera to initialize.
extern VOID InitializeOrthographicCamera(_In_ vec3 Position, _In_ vec4 Rotation,
                                         _Out_ PCAMERA Camera);

/// @brief Updates a camera's matrices if necessary.
///
/// @param Camera The camera to update.
extern VOID CalculateCameraMatrices(_Inout_ PCAMERA Camera);
