#include "swrast.h"

VOID SwrsSetPixel(_In_ UINT32 X, _In_ UINT32 Y, _In_ UINT32 Pixel)
{
    if (X < SwrsData.Framebuffer->Width && Y < SwrsData.Framebuffer->Height)
    {
        SwrsData.Framebuffer->Pixels[X + Y * SwrsData.Framebuffer->Width] = Pixel;
    }
}

VOID SwrsDrawLine(_In_ ivec2 Start, _In_ ivec2 End, _In_ UINT32 Pixel)
{
    BOOLEAN Steep = FALSE;
    if (abs(Start[0] - End[0]) < abs(Start[1] - End[1]))
    {
        PURPL_SWAP(INT32, Start[0], Start[1]);
        PURPL_SWAP(INT32, End[0], End[1]);
        Steep = TRUE;
    }

    if (Start[0] > End[0])
    {
        PURPL_SWAP(INT32, Start[0], End[0]);
        PURPL_SWAP(INT32, Start[1], End[1]);
    }

    INT32 DeltaX = End[0] - Start[0];
    INT32 DeltaX2 = DeltaX * 2;
    INT32 DeltaY = End[1] - Start[1];
    INT32 DeltaError2 = abs(DeltaY) * 2;
    INT32 Error2 = 0;
    INT32 Y = Start[1];
    if (Steep)
    {
        for (INT32 X = Start[0]; X <= End[0]; X++)
        {
            SwrsSetPixel(Y, X, Pixel);
            Error2 += DeltaError2;
            if (Error2 > DeltaX)
            {
                Y += (End[1] > Start[1] ? 1 : -1);
                Error2 -= DeltaX2;
            }
        }
    }
    else
    {
        for (INT32 X = Start[0]; X <= End[0]; X++)
        {
            SwrsSetPixel(X, Y, Pixel);
            Error2 += DeltaError2;
            if (Error2 > DeltaX)
            {
                Y += (End[1] > Start[1] ? 1 : -1);
                Error2 -= DeltaX2;
            }
        }
    }
}

VOID SwrsDrawModel(_In_ PMODEL Model, _In_ mat4 Object, _In_ mat4 World, _In_ mat4 Projection)
{
    PMESH Mesh = Model->MeshHandle;
    DOUBLE HalfWidth = SwrsData.Framebuffer->Width * 0.5;
    DOUBLE HalfHeight = SwrsData.Framebuffer->Height * 0.5;

    if (!Mesh->IndexCount)
    {
        return;
    }

    for (UINT64 i = 0; i < Mesh->IndexCount; i++)
    {
        ivec3 Face;
        glm_ivec3_copy(Mesh->Indices[i], Face);
        for (UINT8 j = 0; j < 3; j++)
        {
            PVERTEX V0 = &Mesh->Vertices[Face[j]];
            PVERTEX V1 = &Mesh->Vertices[Face[(j + 1) % 3]];
            mat4 MVP;
            ivec2 Start;
            ivec2 End;
            vec4 Position0;
            vec4 Position1;
            glm_mat4_mul(Projection, World, MVP);
            glm_mat4_mul(MVP, Object, MVP);
            glm_mat4_mulv(MVP, V0->Position, Position0);
            glm_mat4_mulv(MVP, V1->Position, Position1);
            Start[0] = (Position0[0] + 1.0) * HalfWidth;
            Start[1] = (Position0[1] + 1.0) * HalfHeight;
            End[0] = (Position1[0] + 1.0) * HalfWidth;
            End[1] = (Position1[1] + 1.0) * HalfHeight;
            SwrsDrawLine(Start, End, SWRS_PIXEL(255, 255, 255, 255));
        }
    }
}
