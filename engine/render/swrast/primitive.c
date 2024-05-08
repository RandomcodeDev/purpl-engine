#include "swrast.h"

VOID SwrsPutPixel(_In_ CONST ivec2 Position, _In_ CONST vec4 Colour)
{
    UINT32 X = Position[0];
    UINT32 Y = Position[1];
    SIZE_T Offset = Y * SwrsData.Framebuffer->Width + X;
    UINT32 ColourRaw = VIDEO_PACK_COLOUR(Colour);

    if (X < SwrsData.Framebuffer->Width && Y < SwrsData.Framebuffer->Height)
    {
        SwrsData.Framebuffer->Pixels[Offset] = VidConvertPixel(ColourRaw);
    }
}

static PFLOAT Interpolate(_In_ vec2 Independent, _In_ vec2 Dependent)
{
    PFLOAT Values = NULL;

    if (Independent[0] == Independent[1])
    {
        stbds_arrpush(Values, Dependent[0]);
    }

    FLOAT Slope = (Dependent[1] - Dependent[0]) / (Independent[1] - Independent[0]);
    FLOAT D = Dependent[0];
    for (FLOAT I = Independent[0]; I < Independent[1]; I++)
    {
        stbds_arrpush(Values, D);
        D += Slope;
    }

    return Values;
}

VOID SwrsDrawLine(_In_ CONST vec3 Start, _In_ CONST vec3 End, _In_ CONST vec4 Colour, _In_opt_ CONST mat4 Transform, _In_ BOOLEAN Project)
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

    UINT32 X0 = (UINT32)(StartReal[0] * RdrGetWidth());
    UINT32 Y0 = (UINT32)(StartReal[1] * RdrGetHeight());

    UINT32 X1 = (UINT32)(EndReal[0] * RdrGetWidth());
    UINT32 Y1 = (UINT32)(EndReal[1] * RdrGetHeight());

    INT32 DistanceX = X1 - X0;
    INT32 DistanceY = Y1 - Y0;

    PFLOAT Values;

    if (abs(DistanceX) > abs(DistanceY))
    {
        if (X0 > X1)
        {
            PURPL_SWAP(UINT32, X0, X1);
            PURPL_SWAP(UINT32, Y0, Y1);
        }

        Values = Interpolate((vec2){X0, X1}, (vec2){Y0, Y1});
        for (UINT32 X = 0; X < stbds_arrlenu(Values); X++)
        {
            SwrsPutPixel((vec2){X, Values[X - X0]}, Colour);
        }
    }
    else
    {
        if (Y0 > Y1)
        {
            PURPL_SWAP(FLOAT, X0, X1);
            PURPL_SWAP(FLOAT, Y0, Y1);
        }

        Values = Interpolate((vec2){Y0, Y1}, (vec2){X0, X1});
        for (UINT32 Y = 0; Y < stbds_arrlenu(Values); Y++)
        {
            SwrsPutPixel((vec2){Y, Values[Y - Y0]}, Colour);
        }
    }

    stbds_arrfree(Values);
}

VOID SwrsDrawTriangle(_In_ )
{}
