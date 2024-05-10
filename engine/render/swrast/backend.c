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

static VOID BeginFrame(_In_ BOOLEAN Resized, _In_ CONST PRENDER_SCENE_UNIFORM Uniform)
{
    vec4 ClearColour = {0};
    VIDEO_UNPACK_COLOUR(ClearColour, CONFIGVAR_GET_INT("rdr_clear_colour"));

    for (UINT32 Y = 0; Y < SwrsData.Framebuffer->Height; Y++)
    {
        for (UINT32 X = 0; X < SwrsData.Framebuffer->Width; X++)
        {
            SwrsPutPixel((ivec2){X, Y}, ClearColour);
        }
    }

    glm_mat4_mul(Uniform->Projection, Uniform->View, SwrsData.ViewProjection);
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

static PCSTR GetGpuName(VOID)
{
    return PlatGetCpuName();
}

VOID SwrsInitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->Shutdown = Shutdown;

    Backend->DrawModel = SwrsDrawModel;
    Backend->DrawLine = SwrsDrawLine;

    Backend->GetGpuName = GetGpuName;
}
