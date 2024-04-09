#include "opengl.h"

static VOID Initialize(VOID)
{
    
}

static VOID BeginFrame(_In_ BOOLEAN Resized, _In_ PRENDER_SCENE_UNIFORM Uniform)
{
    
}

static VOID EndFrame(VOID)
{
    
}

VOID GlInitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    LogDebug("Filling out render backend for OpenGL");

    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;

    Backend->LoadShader = GlLoadShader;
    Backend->DestroyShader = GlDestroyShader;

    Backend->CreateModel = GlCreateModel;
    Backend->DrawModel = GlDrawModel;
    Backend->DestroyModel = GlDestroyModel;
}
