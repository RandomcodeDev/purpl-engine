/// @file mathutil.h
///
/// @brief This file defines math utilities
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

/// @brief Create a transformation matrix
///
/// @param[in] Position The translation to apply (default is 0, 0, 0)
/// @param[in] Rotation The rotation to apply (default is 0, 0, 0, 0)
/// @param[in] Scale The scaling to apply (default is 1, 1, 1)
/// @param[out] Transform The transformation matrix to fill
static inline VOID MthCreateTransformMatrix(_In_opt_ CONST vec3 Position, _In_opt_ CONST vec4 Rotation, _In_opt_ CONST vec3 Scale,
                                            _Out_ mat4 Transform)
{
    vec3 DefaultPosition = {0.0f, 0.0f, 0.0f};
    vec4 DefaultRotation = {0.0f, 0.0f, 0.0f, 0.0f};
    vec3 DefaultScale = {1.0f, 1.0f, 1.0f};

    glm_mat4_identity(Transform);
    glm_translate(Transform, Position ? Position : DefaultPosition);
    glm_rotate(Transform, glm_rad(Rotation ? Rotation[3] : DefaultRotation[3]), Rotation ? Rotation : DefaultRotation);
    glm_scale(Transform, Scale ? Scale : DefaultScale);
}
