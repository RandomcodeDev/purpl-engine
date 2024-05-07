#include "swrast.h"

VOID SwrsDrawLine(_In_ vec3 Start, _In_ vec3 End, _In_ vec4 Colour, _In_opt_ mat4 Transform, _In_ BOOLEAN Project)
{
    vec4 StartReal;
    vec4 EndReal;

    glm_vec3_copy(Start, StartReal);
    glm_vec3_copy(End, EndReal);

    if (Transform)
    {
        glm_mat4_mulv(Transform, StartReal, StartReal);
        glm_mat4_mulv(Transform, EndReal, EndReal);
    }
    if (Project)
    {
        glm_mat4_mulv(SwrsData.ViewProjection, StartReal, StartReal);
        glm_mat4_mulv(SwrsData.ViewProjection, EndReal, EndReal);
    }


}
