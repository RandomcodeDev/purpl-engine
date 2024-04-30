#include "mathutil.h"

VOID MthCreateTransformMatrix(_In_opt_ vec3 Position, _In_opt_ vec4 Rotation, _In_opt_ vec3 Scale, _Out_ mat4 Transform)
{
    CONST vec3 DefaultPosition = {0.0f, 0.0f, 0.0f};
    CONST vec4 DefaultRotation = {0.0f, 0.0f, 0.0f, 0.0f};
    CONST vec3 DefaultScale = {1.0f, 1.0f, 1.0f};

    glm_mat4_identity(Transform);
    glm_translate(Transform, Position ? Position : DefaultPosition);
    glm_rotate(Transform, glm_rad(Rotation ? Rotation[3] : DefaultRotation[3]), Rotation ? Rotation : DefaultRotation);
    glm_scale(Transform, Scale ? Scale : DefaultScale);
}
