#include "swrast.h"

static BOOLEAN IsPointOnScreen(_In_ UINT32 X, _In_ UINT32 Y)
{
    return X < SwrsData.Framebuffer->Width && Y < SwrsData.Framebuffer->Height;
}

VOID SwrsProjectVertex(_Inout_ PMESH_VERTEX Vertex, _In_ mat4 Transform)
{
    vec4 Position;

    glm_vec3_copy(Vertex->Position, Position);
    glm_mat4_mulv(SwrsData.ViewProjection, Position, Position);
    glm_mat4_mulv(Transform, Position, Position);
    glm_vec3_copy(Position, Vertex->Position);
}

VOID SwrsPutPixel(_In_ CONST ivec2 Position, _In_ CONST vec4 Colour)
{
    UINT32 X = Position[0];
    UINT32 Y = Position[1];
    SIZE_T Offset = Y * SwrsData.Framebuffer->Width + X;
    UINT32 ColourRaw = VIDEO_PACK_COLOUR(Colour);

    if (IsPointOnScreen(X, Y))
    {
        SwrsData.Framebuffer->Pixels[Offset] = VidConvertPixel(ColourRaw);
    }
}

static PFLOAT Interpolate(_In_ FLOAT I0, _In_ FLOAT I1, _In_ FLOAT D0, _In_ FLOAT D1)
{
    PFLOAT Values = NULL;

    if (I0 == I1)
    {
        stbds_arrpush(Values, (FLOAT)D0);
        return Values;
    }

    stbds_arrsetcap(Values, (UINT32)(I1 - I0));

    FLOAT Slope = ((FLOAT)D1 - (FLOAT)D0) / ((FLOAT)I1 - (FLOAT)I0);
    FLOAT D = D0;
    for (FLOAT I = I0; I < I1; I++)
    {
        stbds_arrpush(Values, D);
        D += Slope;
    }

    return Values;
}

VOID SwrsDrawLine(_In_ MESH_VERTEX Start, _In_ MESH_VERTEX End, _In_opt_ CONST mat4 Transform,
                  _In_ BOOLEAN Project)
{
    vec4 StartReal;
    vec4 EndReal;

    glm_vec3_copy(Start.Position, StartReal);
    glm_vec3_copy(End.Position, EndReal);

    if (Project)
    {
        glm_mat4_mulv(SwrsData.ViewProjection, StartReal, StartReal);
        glm_mat4_mulv(SwrsData.ViewProjection, EndReal, EndReal);
    }
    if (Transform)
    {
        glm_mat4_mulv(Transform, StartReal, StartReal);
        glm_mat4_mulv(Transform, EndReal, EndReal);
    }

    FLOAT X0 = StartReal[0] * RdrGetWidth();
    FLOAT Y0 = StartReal[1] * RdrGetHeight();

    FLOAT X1 = EndReal[0] * RdrGetWidth();
    FLOAT Y1 = EndReal[1] * RdrGetHeight();

    FLOAT ChangeX = X1 - X0;
    FLOAT ChangeY = Y1 - Y0;

    PFLOAT Values;

    if (abs(ChangeX) > abs(ChangeY))
    {
        if (X0 > X1)
        {
            PURPL_SWAP(FLOAT, X0, X1);
            PURPL_SWAP(FLOAT, Y0, Y1);
        }

        Values = Interpolate(X0, X1, Y0, Y1);
        for (FLOAT X = X0; X < X1; X++)
        {
            SwrsPutPixel((ivec2){(UINT32)X, (UINT32)Values[(UINT32)(X - X0)]}, Start.Colour);
        }
    }
    else
    {
        if (X0 > X1)
        {
            PURPL_SWAP(FLOAT, X0, X1);
            PURPL_SWAP(FLOAT, Y0, Y1);
        }

        Values = Interpolate(Y0, Y1, X0, X1);
        for (FLOAT Y = Y0; Y < Y1; Y++)
        {
            SwrsPutPixel((ivec2){(UINT32)Values[(UINT32)(Y - Y0)], (UINT32)Y}, Start.Colour);
        }
    }

    stbds_arrfree(Values);
}

VOID SwrsDrawTriangle(_In_ CONST MESH_VERTEX First, _In_ CONST MESH_VERTEX Second, _In_ CONST MESH_VERTEX Third,
                      _In_ BOOLEAN Filled)
{
    if (Filled)
    {
        FLOAT X0 = First.Position[0] * RdrGetWidth();
        FLOAT X1 = Second.Position[0] * RdrGetWidth();
        FLOAT X2 = Third.Position[0] * RdrGetWidth();
        FLOAT Y0 = First.Position[1] * RdrGetHeight();
        FLOAT Y1 = Second.Position[1] * RdrGetHeight();
        FLOAT Y2 = Third.Position[1] * RdrGetHeight();

        if (Y1 < Y0)
        {
            PURPL_SWAP(FLOAT, X1, X0);
            PURPL_SWAP(FLOAT, Y1, Y0);
        }
        if (Y2 < Y0)
        {
            PURPL_SWAP(FLOAT, X2, X0);
            PURPL_SWAP(FLOAT, Y2, Y0);
        }
        if (Y2 < Y1)
        {
            PURPL_SWAP(FLOAT, X2, X1);
            PURPL_SWAP(FLOAT, Y2, Y1);
        }

        PFLOAT X01 = Interpolate(Y0, Y1, X0, X1);
        PFLOAT X12 = Interpolate(Y1, Y2, X1, X2);
        PFLOAT X02 = Interpolate(Y0, Y2, X0, X2);

        stbds_arrpop(X01);
        PFLOAT X012 = NULL;
        stbds_arrappend(X012, X01);
        stbds_arrappend(X012, X12);
        stbds_arrfree(X01);
        stbds_arrfree(X12);

        SIZE_T Middle = stbds_arrlenu(X012) / 2;
        PFLOAT Left = X012;
        PFLOAT Right = X02;
        if (X02[Middle] < X012[Middle])
        {
            Left = X02;
            Right = X012;
        }

        for (FLOAT Y = Y0; Y < Y2; Y++)
        {
            FLOAT LeftX = Left[(UINT32)(Y - Y0)];
            FLOAT RightX = Right[(UINT32)(Y - Y0)];
            for (FLOAT X = LeftX; X < RightX; X++)
            {
                SwrsPutPixel((ivec2){(UINT32)X, (UINT32)Y}, First.Colour);
            }
        }

        stbds_arrfree(X02);
        stbds_arrfree(X012);
    }
    else
    {
        SwrsDrawLine(First, Second, NULL, FALSE);
        SwrsDrawLine(Second, Third, NULL, FALSE);
        SwrsDrawLine(Third, First, NULL, FALSE);
    }
}
