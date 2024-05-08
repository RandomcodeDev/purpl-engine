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

static VOID BeginFrame(_In_ BOOLEAN Resized)
{
    // Clear all colours of all pixels to 0
    memset(SwrsData.Framebuffer->Pixels, 0,
           SwrsData.Framebuffer->Width * SwrsData.Framebuffer->Height * sizeof(UINT32));

    SwrsDrawLine((vec2){1.0, 0.0}, (vec2){0.0, 1.0}, (vec4){0.5, 0, 1.0, 1.0}, NULL, FALSE);
}

static VOID EndFrame(VOID)
{
    VidDisplayFramebuffer(SwrsData.Framebuffer);
}

static VOID Shutdown(VOID)
{
    LogDebug("Shutting down software rasterizer");

    VidDestroyFramebuffer(SwrsData.Framebuffer);

    LogDebug("Software rasterizer shutdown succeeded");
}

VOID SwrsInitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->Shutdown = Shutdown;

    Backend->DrawLine = SwrsDrawLine;
}
