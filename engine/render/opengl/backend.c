#include "opengl.h"

OPENGL_DATA GlData;

static VOID Initialize(VOID)
{
    LogInfo("Initializing OpenGL");

    GlSetDebugCallback();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (INT32 *)&GlData.UniformBufferAlignment);

    GlData.UniformBuffer =
        GlCreateUniformBuffer(PURPL_ALIGN(GlData.UniformBufferAlignment, sizeof(RENDER_SCENE_UNIFORM)) +
                              PURPL_ALIGN(GlData.UniformBufferAlignment, sizeof(RENDER_OBJECT_UNIFORM)));

    LogInfo("Successfully initialized OpenGL");
}

PURPL_MAKE_STRING_HASHMAP_ENTRY(SHADERMAP, RENDER_HANDLE);
extern PSHADERMAP RdrShaders;

static VOID BeginFrame(_In_ BOOLEAN Resized, _In_ PRENDER_SCENE_UNIFORM Uniform)
{
    UINT64 ClearColourRaw = CONFIGVAR_GET_INT("rdr_clear_colour");
    vec4 ClearColour;
    ClearColour[0] = (UINT8)((ClearColourRaw >> 24) & 0xFF) / 255.0f;
    ClearColour[1] = (UINT8)((ClearColourRaw >> 16) & 0xFF) / 255.0f;
    ClearColour[2] = (UINT8)((ClearColourRaw >> 8) & 0xFF) / 255.0f;
    ClearColour[3] = (UINT8)((ClearColourRaw >> 0) & 0xFF) / 255.0f;

    glClearColor(ClearColour[0], ClearColour[1], ClearColour[2], ClearColour[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, RdrGetWidth(), RdrGetHeight());
    glScissor(0, 0, RdrGetWidth(), RdrGetHeight());

    GlWriteUniformBuffer(GlData.UniformBuffer, 0, Uniform, sizeof(RENDER_SCENE_UNIFORM));
}

static VOID EndFrame(VOID)
{
}

static PCSTR GetGpuName(VOID)
{
    return (PCSTR)glGetString(GL_RENDERER);
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

    Backend->GetGpuName = GetGpuName;
}
