#include "swrast.h"

VOID SwrsPutPixel(_In_ vec2 Position, _In_ vec4 Colour)
{
    UINT32 X = Position[0] * SwrsData.Framebuffer->Width;
    UINT32 Y = Position[1] * SwrsData.Framebuffer->Height;
    UINT32 Offset = Y * SwrsData.Framebuffer->Width + X;
    UINT32 Colour = VIDEO_PACK_COLOUR(Colour);

    SwrsData.Framebuffer->Pixels[Offset] = VidConvertPixel(Colour);
}
