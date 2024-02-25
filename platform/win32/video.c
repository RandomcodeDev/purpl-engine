/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    video.c

Abstract:

    This file implements the Windows video functions.

--*/

#include "common/alloc.h"
#include "common/common.h"

#include "platform/platform.h"
#include "platform/video.h"

// #include "imgui_backends/imgui_impl_win32.h"

#ifdef _MSC_VER
#pragma comment(                                                                                                       \
    linker,                                                                                                            \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define IDI_ICON1 103

static HWND Window = NULL;

static CHAR WindowClassName[] = "PurplWindow";

#ifdef PURPL_DEBUG
static CHAR WindowTitle[128] = PURPL_NAME " v" PURPL_VERSION_STRING " commit " PURPL_BRANCH "-" PURPL_COMMIT;
#else
static CHAR WindowTitle[128] = PURPL_NAME " v" PURPL_VERSION_STRING;
#endif
static INT32 WindowWidth;
static INT32 WindowHeight;
static INT32 ExtraWidth;
static INT32 ExtraHeight;

static BOOLEAN WindowResized;
static BOOLEAN WindowFocused;
static BOOLEAN WindowClosed;

static HDC WindowDeviceContext;

static LRESULT CALLBACK WindowProcedure(_In_ HWND MessageWindow, _In_ UINT Message, _In_ WPARAM Wparam,
                                        _In_ LPARAM Lparam)
{
    //    ImGuiIO* Io = igGetIO();

    //    if ( ImGui_ImplWin32_WndProcHandler(
    //            MessageWindow,
    //            Message,
    //            Wparam,
    //            Lparam
    //            ) )
    //    {
    //        return TRUE;
    //    }

    if (MessageWindow == Window)
    {
        switch (Message)
        {
        case WM_SETTEXT: {
            strncpy(WindowTitle, (PSTR)Lparam, PURPL_ARRAYSIZE(WindowTitle));
            return 0;
        }
        case WM_SIZE: {
            RECT ClientArea = {0};
            INT32 NewWidth;
            INT32 NewHeight;

            GetClientRect(Window, &ClientArea);
            NewWidth = ClientArea.right - ClientArea.left;
            NewHeight = ClientArea.bottom - ClientArea.top;

            if (NewWidth != WindowWidth || NewHeight != WindowHeight)
            {
                WindowResized = TRUE;
                LogInfo("Window resized from %dx%d to %dx%d", WindowWidth, WindowHeight, NewWidth, NewHeight);
            }
            WindowWidth = NewWidth;
            WindowHeight = NewHeight;
            return 0;
        }
        case WM_ACTIVATEAPP:
            WindowFocused = (BOOLEAN)Wparam;
            LogInfo("Window %s", WindowFocused ? "focused" : "unfocused");
            return 0;
        case WM_DESTROY:
        case WM_CLOSE: {
            LogInfo("Window closed");
            WindowClosed = TRUE;
            return 0;
        }
        }
    }

    return DefWindowProcA(MessageWindow, Message, Wparam, Lparam);
}

static VOID RegisterWindowClass(VOID)
{
    WNDCLASSEXA WindowClass;

    LogDebug("Registering window class %s", WindowClassName);

    memset(&WindowClass, 0, sizeof(WindowClass));
    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.lpfnWndProc = WindowProcedure;
    WindowClass.hInstance = GetModuleHandleA(NULL);
    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.hIcon = LoadIconA(GetModuleHandleA(NULL), (PCSTR)IDI_ICON1);
    WindowClass.lpszClassName = WindowClassName;
    if (!RegisterClassExA(&WindowClass))
    {
        DWORD Error = GetLastError();
        CmnError("Failed to register window class: error 0x%X (%d)", Error, Error);
    }

    LogDebug("Window class registered");
}

static VOID InitializeWindow(VOID)
{
    RECT ClientArea;
    RECT AdjustedClientArea;

    ClientArea.left = 0;
    ClientArea.right = (UINT32)(GetSystemMetrics(SM_CXSCREEN) / 1.5);
    ClientArea.top = 0;
    ClientArea.bottom = (UINT32)(GetSystemMetrics(SM_CYSCREEN) / 1.5);
    AdjustedClientArea = ClientArea;
    AdjustWindowRect(&AdjustedClientArea, WS_OVERLAPPEDWINDOW, FALSE);
    WindowWidth = AdjustedClientArea.right - AdjustedClientArea.left;
    WindowHeight = AdjustedClientArea.bottom - AdjustedClientArea.top;

    ExtraWidth = ClientArea.left - AdjustedClientArea.left;
    ExtraHeight = ClientArea.top - AdjustedClientArea.top;

    LogDebug("Creating %dx%d (for internal size %dx%d) window titled %s", WindowWidth, WindowHeight,
             ClientArea.right - ClientArea.left, ClientArea.bottom - ClientArea.top, WindowTitle);

    Window = CreateWindowExA(0, WindowClassName, WindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             WindowWidth, WindowHeight, NULL, NULL, GetModuleHandleA(NULL), NULL);
    if (!Window)
    {
        CmnError("Failed to create window: error 0x%X (%d)", GetLastError(), GetLastError());
    }

    GetClientRect(Window, &ClientArea);
    WindowWidth = ClientArea.right - ClientArea.left;
    WindowHeight = ClientArea.bottom - ClientArea.top;

    WindowResized = FALSE;
    WindowFocused = TRUE;
    WindowClosed = FALSE;

    WindowDeviceContext = GetWindowDC(Window);

    LogDebug("Successfully created window with handle 0x%llX", (UINT64)Window);
}

VOID VidInitialize(VOID)
{
    LogInfo("Initializing Windows video");

    RegisterWindowClass();
    InitializeWindow();

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    // ImGui_ImplWin32_Init(Window);

    LogDebug("Showing window");
    ShowWindow(Window, SW_SHOWDEFAULT);
}

BOOLEAN VidUpdate(VOID)
{
    MSG Message;

    // ImGui_ImplWin32_NewFrame();

    while (PeekMessageA(&Message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }

    // Set in the window procedure
    return !WindowClosed;
}

VOID VidShutdown(VOID)
{
    LogInfo("Shutting down Windows video");

    // ImGui_ImplWin32_Shutdown();

    ReleaseDC(Window, WindowDeviceContext);

    LogDebug("Destroying window");
    DestroyWindow(Window);

    LogInfo("Successfully shut down Windows video");
}

VOID VidGetSize(_Out_opt_ PUINT32 Width, _Out_opt_ PUINT32 Height)
{
    Width ? *Width = WindowWidth : 0;
    Height ? *Height = WindowHeight : 0;
}

BOOLEAN VidResized(VOID)
{
    BOOLEAN ReturnValue = WindowResized;
    WindowResized = FALSE;
    return ReturnValue;
}

BOOLEAN VidFocused(VOID)
{
    return WindowFocused;
}

PVOID VidGetObject(VOID)
{
    return Window;
}

FLOAT VidGetDpi(VOID)
{
    return (FLOAT)GetDpiForWindow(Window);
}

#ifdef PURPL_VULKAN
VkSurfaceKHR VidCreateVulkanSurface(_In_ VkInstance Instance, _In_ PVOID AllocationCallbacks,
                                    _In_opt_ PVOID WindowHandle)
{
    VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {0};
    VkSurfaceKHR Surface;
    VkResult Result;

    LogDebug("Creating Vulkan surface with vkCreateWin32SurfaceKHR");

    SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    SurfaceCreateInfo.hinstance = GetModuleHandleA(NULL);
    SurfaceCreateInfo.hwnd = WindowHandle ? WindowHandle : Window;

    Surface = NULL;
    Result = vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, AllocationCallbacks, &Surface);
    if (Result != VK_SUCCESS)
    {
        LogError("Failed to create Vulkan surface: VkResult %d", Result);
        return VK_NULL_HANDLE;
    }

    LogDebug("Successfully created Vulkan surface with handle 0x%llX", (UINT64)Surface);

    return Surface;
}
#endif

typedef struct WINDOWS_FRAMEBUFFER_DATA
{
    HDC DeviceContext;
    HBITMAP Bitmap;
    BITMAPINFO BitmapInfo;
} WINDOWS_FRAMEBUFFER_DATA, *PWINDOWS_FRAMEBUFFER_DATA;

static BOOLEAN CreateFramebufferBitmap(_In_ PVIDEO_FRAMEBUFFER Framebuffer,
                                       _Inout_ PWINDOWS_FRAMEBUFFER_DATA FramebufferData)
{
    DWORD Error;

    FramebufferData->Bitmap = CreateDIBSection(FramebufferData->DeviceContext, &FramebufferData->BitmapInfo, 0,
                                               &Framebuffer->Pixels, NULL, 0);
    if (!FramebufferData->Bitmap)
    {
        Error = GetLastError();
        LogError("Failed to create bitmap: %d (0x%X)", Error, Error);
        ReleaseDC(Window, FramebufferData->DeviceContext);
        CmnFree(Framebuffer);
        return FALSE;
    }

    SelectObject(FramebufferData->DeviceContext, FramebufferData->Bitmap);

    return TRUE;
}

PVIDEO_FRAMEBUFFER VidCreateFramebuffer(VOID)
{
    PVIDEO_FRAMEBUFFER Framebuffer;
    PWINDOWS_FRAMEBUFFER_DATA FramebufferData;

    if (!Window)
    {
        LogWarning("Not creating framebuffer without window");
        return NULL;
    }

    Framebuffer = CmnAlloc(1, sizeof(VIDEO_FRAMEBUFFER) + sizeof(WINDOWS_FRAMEBUFFER_DATA));
    if (!Framebuffer)
    {
        LogError("Failed to allocate framebuffer structure");
        return NULL;
    }

    Framebuffer->Handle = Framebuffer + 1;
    FramebufferData = Framebuffer->Handle;

    Framebuffer->Width = WindowWidth;
    Framebuffer->Height = WindowHeight;

    PBITMAPINFO BitmapInfo = &FramebufferData->BitmapInfo;
    BitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitmapInfo->bmiHeader.biWidth = Framebuffer->Width;
    BitmapInfo->bmiHeader.biHeight = Framebuffer->Height;
    BitmapInfo->bmiHeader.biCompression = BI_RGB;
    BitmapInfo->bmiHeader.biBitCount = 32;
    BitmapInfo->bmiHeader.biPlanes = 1;

    FramebufferData->DeviceContext = CreateCompatibleDC(WindowDeviceContext);

    if (!CreateFramebufferBitmap(Framebuffer, FramebufferData))
    {
        return NULL;
    }

    return Framebuffer;
}

VOID VidDisplayFramebuffer(_Inout_ PVIDEO_FRAMEBUFFER Framebuffer)
{
    if (!Framebuffer)
    {
        return;
    }

    PWINDOWS_FRAMEBUFFER_DATA FramebufferData = Framebuffer->Handle;
    StretchDIBits(WindowDeviceContext, ExtraWidth, WindowHeight + ExtraHeight, WindowWidth, -WindowHeight, 0, 0,
                  Framebuffer->Width, Framebuffer->Height, Framebuffer->Pixels, &FramebufferData->BitmapInfo,
                  DIB_RGB_COLORS, SRCCOPY);

    if (Framebuffer->Width != (UINT32)WindowWidth || Framebuffer->Height != (UINT32)WindowHeight)
    {
        PWINDOWS_FRAMEBUFFER_DATA FramebufferData = Framebuffer->Handle;
        DeleteObject(FramebufferData->Bitmap);

        Framebuffer->Width = WindowWidth;
        Framebuffer->Height = WindowHeight;

        PBITMAPINFO BitmapInfo = &FramebufferData->BitmapInfo;
        BitmapInfo->bmiHeader.biWidth = Framebuffer->Width;
        BitmapInfo->bmiHeader.biHeight = Framebuffer->Height;

        PURPL_ASSERT(CreateFramebufferBitmap(Framebuffer, FramebufferData));
    }
}

VOID VidDestroyFramebuffer(_In_ PVIDEO_FRAMEBUFFER Framebuffer)
{
    PWINDOWS_FRAMEBUFFER_DATA FramebufferData = Framebuffer->Handle;
    DeleteObject(FramebufferData->Bitmap);
    DeleteDC(FramebufferData->DeviceContext);
    CmnFree(Framebuffer);
}

UINT32 VidConvertPixel(_In_ UINT32 Pixel)
{
    // Rotate RGBA to ARGB
    return _rotr(Pixel, 8);
}
