#include "swrast.h"

SOFTWARE_RASTERIZER_DATA SwrsData;

static VOID Initialize(VOID)
{
    LogDebug("Initializing software rasterizer");

    LogDebug("Creating framebuffer");
    PURPL_ASSERT(SwrsData.Framebuffer = VidCreateFramebuffer());

    LogDebug("Software rasterizer initialization succeeded");
}

static VOID BeginFrame(_In_ BOOLEAN Resized)
{
    SwrsDrawLine((ivec2){0, 0}, (ivec2){500, 100}, SWRS_PIXEL(135, 0, 255, 255));
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
}
