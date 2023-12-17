/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    mathutil.h

Abstract:

    This module defines various math utility functions.

--*/

#pragma once

#include "purpl/purpl.h"

#include "common/log.h"

//
// A transformation
//

typedef struct TRANSFORM
{
    vec3 Position;
    vec4 Rotation;
    vec3 Scale;
} TRANSFORM, *PTRANSFORM;
extern ECS_COMPONENT_DECLARE(TRANSFORM);

//
// Convert a transform into a transformation matrix
//

extern
VOID
MathCreateTransformMatrix(
    _In_ PTRANSFORM Transform,
    _Out_ mat4 TransformMatrix
    );

//
// Convert Euler angles into an axis angle
//

extern
VOID
MathEulerToAxisAngle(
    _In_ vec3 Euler,
    _Out_ vec4 AxisAngle
    );
