/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    mathutil.h

Abstract:

    This module defines structures and functions for cameras.

--*/

#pragma once

#include "purpl/purpl.h"

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

//
// Initialize a perspective camera
//

extern
VOID
InitializePerspectiveCamera(
    _In_ vec3 Position,
    _In_ vec4 Rotation,
    _In_ DOUBLE FieldOfView,
    _In_ DOUBLE Aspect,
    _In_ DOUBLE NearClip,
    _In_ DOUBLE FarClip,
    _Out_ PCAMERA Camera
    );

//
// Initialize an orthographic camera
//

extern
VOID
InitializeOrthographicCamera(
    _In_ vec3 Position,
    _In_ vec4 Rotation,
    _Out_ PCAMERA Camera
    );

//
// Calculate the camera matrices
//

extern
VOID
CalculateCameraMatrices(
    _Inout_ PCAMERA Camera
    );
