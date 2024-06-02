/// @file mathutil.h
///
/// @brief This file defines math utilities
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "common/common.h"

/// @brief Create a transformation matrix
///
/// @param[in] Position The translation to apply (default is 0, 0, 0)
/// @param[in] Rotation The rotation to apply (default is 0, 0, 0)
/// @param[in] Scale The scaling to apply (default is 1, 1, 1)
/// @param[out] Transform The transformation matrix to fill
static inline VOID MthCreateTransformMatrix(_In_opt_ CONST vec3 Position, _In_opt_ CONST vec3 Rotation,
                                            _In_opt_ CONST vec3 Scale, _Out_ mat4 Transform)
{
    vec3 DefaultPosition = {0.0f, 0.0f, 0.0f};
    vec3 DefaultRotation = {0.0f, 0.0f, 0.0f};
    vec3 DefaultScale = {1.0f, 1.0f, 1.0f};

    glm_mat4_identity(Transform);
    glm_translate(Transform, (PFLOAT)(Position ? Position : DefaultPosition));
    glm_scale(Transform, (PFLOAT)(Scale ? Scale : DefaultScale));

    mat4 RotationMatrix;
    glm_euler((PFLOAT)(Rotation ? Rotation : DefaultRotation), RotationMatrix);
    glm_mat4_mul(Transform, RotationMatrix, Transform);
}
