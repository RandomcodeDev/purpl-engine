/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    mathutil.c

Abstract:

    This file implements various math utility functions.

--*/

#include "mathutil.h"

ecs_entity_t ecs_id(TRANSFORM);

VOID MathImport(_In_ ecs_world_t *World)
{
    LogTrace("Importing Math ECS module");
    ECS_MODULE(World, Math);

    ECS_COMPONENT_DEFINE(World, TRANSFORM);
}

VOID MthCreateTransformMatrix(_In_ PTRANSFORM Transform,
                              _Out_ mat4 TransformMatrix)
{
    if (!Transform || !TransformMatrix)
    {
        return;
    }

    glm_mat4_identity(TransformMatrix);
    glm_translate(TransformMatrix, Transform->Position);
    glm_rotate(TransformMatrix, glm_rad(Transform->Rotation[3]),
               Transform->Rotation);
    glm_scale(TransformMatrix, Transform->Scale);
}

VOID MthEulerToAxisAngle(_In_ vec3 Euler, _Out_ vec4 AxisAngle)
{
    // https://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToAngle/
    float c1 = cos(Euler[0]) / 2;
    float s1 = sin(Euler[0]) / 2;
    float c2 = cos(Euler[1]) / 2;
    float s2 = sin(Euler[1]) / 2;
    float c3 = cos(Euler[2]) / 2;
    float s3 = sin(Euler[2]) / 2;

    AxisAngle[3] = 2 * acos(c1 * c2 * c3 - s1 * s2 * s3); // angle
    AxisAngle[0] = s1 * s2 * c3 + c1 * c2 * s3;           // x
    AxisAngle[1] = s1 * c2 * c3 + c1 * s2 * s3;           // y
    AxisAngle[2] = c1 * s2 * c3 - s1 * c2 * s3;           // z
}
