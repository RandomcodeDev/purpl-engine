#include "ui.h"

static ImGuiContext *Context;

EXTERN_C
VOID UiInitialize(VOID)
{
    LogInfo("Initializing UI");

    Context = igCreateContext(NULL);

    ImGuiIO *Io = igGetIO();

    igStyleColorsDark(NULL);

    Io->BackendPlatformName = "Purpl";
    Io->BackendRendererName = "Purpl";

    Io->BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
}

EXTERN_C
VOID UiShutdown(VOID)
{
    LogInfo("Shutting down UI");

    igDestroyContext(Context);
}
