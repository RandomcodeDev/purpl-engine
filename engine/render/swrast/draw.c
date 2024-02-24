#include "swrast.h"

VOID SwrsSetPixel(_In_ UINT32 X, _In_ UINT32 Y, _In_ UINT32 Pixel)
{
    SwrsData.Framebuffer->Pixels[X + Y * SwrsData.Framebuffer->Width] = Pixel;
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
