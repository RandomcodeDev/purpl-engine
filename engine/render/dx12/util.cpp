#include "dx12.h"

EXTERN_C
VOID Dx12EnableDebugLayer(VOID)
{
    ID3D12Debug6 *Debug;
    IDXGIDebug1 *DxgiDebug;

    LogDebug("Enabling DXGI debug features");

    HRESULT Result = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DxgiDebug));
    if (SUCCEEDED(Result))
    {
        DxgiDebug->EnableLeakTrackingForThread();
        DxgiDebug->Release();
    }
    else
    {
        _com_error Error(Result);
        LogError("Failed to enable debug features: %s (HRESULT 0x%08X)", Error.ErrorMessage(), Result);
    }

    LogDebug("Enabling DirectX 12 debug features");

    Result = D3D12GetDebugInterface(IID_PPV_ARGS(&Debug));
    if (SUCCEEDED(Result))
    {
        Debug->EnableDebugLayer();
        Debug->SetEnableAutoName(TRUE);
        Debug->Release();
        LogDebug("Debug layer enabled");
    }
    else
    {
        _com_error Error(Result);
        LogError("Failed to enable debug features: %s (HRESULT 0x%08X)", Error.ErrorMessage(), Result);
    }
}

EXTERN_C
VOID Dx12NameObject(_Inout_ ID3D12Object *Object, _In_z_ PCSTR Name, ...)
{
    va_list Arguments;
    va_start(Arguments, Name);
    PSTR NameFormatted = CmnFormatStringVarArgs(Name, Arguments);
    va_end(Arguments);

    SIZE_T Size = strlen(NameFormatted) + 1;
    PWSTR NameWide = (PWSTR)CmnAlloc(Size, sizeof(WCHAR));
    if (!NameWide)
    {
        return;
    }

    mbstowcs(NameWide, NameFormatted, Size);

    CmnFree(NameFormatted);

    Object->SetName(NameWide);

    CmnFree(NameWide);
}

EXTERN_C
VOID Dx12WaitForGpu(VOID)
{
    HRESULT_CHECK(Dx12Data.CommandQueue->Signal(Dx12Data.Fence, Dx12Data.FenceValues[Dx12Data.FrameIndex]));

    HRESULT_CHECK(Dx12Data.Fence->SetEventOnCompletion(Dx12Data.FenceValues[Dx12Data.FrameIndex], Dx12Data.FenceEvent));
    WaitForSingleObjectEx(Dx12Data.FenceEvent, INFINITE, FALSE);

    Dx12Data.FenceValues[Dx12Data.FrameIndex]++;
}
