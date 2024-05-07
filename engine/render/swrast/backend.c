#include "swrast.h"

SWRAST_DATA SwrsData;

static VOID Initialize(VOID)
{
    LogDebug("Initializing software rasteriser");

    SwrsData.Framebuffer = VidCreateFramebuffer();
    if (!SwrsData.Framebuffer)
    {
        CmnError("Failed to create framebuffer");
    }

    LogDebug("Successfully initialized software rasteriser");
}
