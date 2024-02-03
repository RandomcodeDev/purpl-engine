/// @file entity.h
/// 
/// @brief This module defines various math utility functions.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/log.h"

/// @brief A transformation
typedef struct TRANSFORM
{
    vec3 Position;
    vec4 Rotation;
    vec3 Scale;
} TRANSFORM, *PTRANSFORM;
extern ECS_COMPONENT_DECLARE(TRANSFORM);

/// @brief Creates a transform matrix from a TRANSFORM.
///
/// @param[in] Transform        The TRANSFORM to calculate a matrix from.
/// @param[out] TransformMatrix  The transform matrix to calculate into.
extern VOID MthCreateTransformMatrix(_In_ PTRANSFORM Transform,
                                     _Out_ mat4 TransformMatrix);

/// @brief Convert Euler angles into an axis angle.
///
/// @param[in] Euler     The Euler angles to convert.
/// @param[out] AxisAngle The axis angle to store the result in.
extern VOID MthEulerToAxisAngle(_In_ vec3 Euler, _Out_ vec4 AxisAngle);
