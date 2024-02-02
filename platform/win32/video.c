/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    video.c

Abstract:

    This module implements the Windows video functions.

--*/

#include "common/common.h"

#include "platform/platform.h"

// #include "imgui_backends/imgui_impl_win32.h"

#ifdef _MSC_VER
#pragma comment(                                                               \
    linker,                                                                    \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define IDI_ICON1 103

static HWND Window = NULL;

static CHAR WindowClassName[] = "PurplWindow";

#ifdef PURPL_DEBUG
static CHAR WindowTitle[128] =
    GAME_NAME " v" GAME_VERSION_STRING " commit " GAME_BRANCH "-" GAME_COMMIT;
#else
static CHAR WindowTitle[128] = GAME_NAME " v" GAME_VERSION_STRING;
#endif
static INT32 WindowWidth;
static INT32 WindowHeight;

static BOOLEAN WindowResized;
static BOOLEAN WindowFocused;
static BOOLEAN WindowClosed;

static LRESULT CALLBACK WindowProcedure(_In_ HWND MessageWindow,
                                        _In_ UINT Message, _In_ WPARAM Wparam,
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

    if (!Window || MessageWindow == Window)
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
                LogInfo("Window resized from %dx%d to %dx%d", WindowWidth,
                        WindowHeight, NewWidth, NewHeight);
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
        CmnError("Failed to register window class: error 0x%X (%d)",
                 GetLastError, GetLastError);
    }

    LogDebug("Window class registered");
}

static VOID InitializeWindow(VOID)
{
    RECT ClientArea;

    ClientArea.left = 0;
    ClientArea.right = (UINT32)(GetSystemMetrics(SM_CXSCREEN) / 1.5);
    ClientArea.top = 0;
    ClientArea.bottom = (UINT32)(GetSystemMetrics(SM_CYSCREEN) / 1.5);
    AdjustWindowRect(&ClientArea, WS_OVERLAPPEDWINDOW, FALSE);
    WindowWidth = ClientArea.right - ClientArea.left;
    WindowHeight = ClientArea.bottom - ClientArea.top;

    LogDebug("Creating %dx%d window titled %s", WindowWidth, WindowHeight,
             WindowTitle);

    Window =
        CreateWindowExA(0, WindowClassName, WindowTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight,
                        NULL, NULL, GetModuleHandleA(NULL), NULL);
    if (!Window)
    {
        CmnError("Failed to create window: error 0x%X (%d)", GetLastError(),
                 GetLastError());
    }

    GetClientRect(Window, &ClientArea);
    WindowWidth = ClientArea.right - ClientArea.left;
    WindowHeight = ClientArea.bottom - ClientArea.top;

    WindowResized = FALSE;
    WindowFocused = TRUE;
    WindowClosed = FALSE;

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

BOOLEAN
VidUpdate(VOID)
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

    LogDebug("Destroying window");
    DestroyWindow(Window);

    LogInfo("Successfully shut down Windows video");
}

VOID VidGetSize(_Out_opt_ PUINT32 Width, _Out_opt_ PUINT32 Height)
{
    Width ? *Width = WindowWidth : 0;
    Height ? *Height = WindowHeight : 0;
}

BOOLEAN
VidResized(VOID)
{
    BOOLEAN ReturnValue = WindowResized;
    WindowResized = FALSE;
    return ReturnValue;
}

BOOLEAN
VidFocused(VOID)
{
    return WindowFocused;
}

PVOID
VidGetObject(VOID)
{
    return Window;
}

FLOAT
VidGetDpi(VOID)
{
    return (FLOAT)GetDpiForWindow(Window);
}

#ifdef PURPL_VULKAN
PVOID
PlatCreateVulkanSurface(_In_ PVOID Instance, _In_ PVOID AllocationCallbacks,
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
    Result = vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo,
                                     AllocationCallbacks, &Surface);
    if (Result != VK_SUCCESS)
    {
        CmnError("Failed to create VkSurfaceKHR: VkResult %d", Result);
    }

    LogDebug("Successfully created Vulkan surface with handle 0x%llX",
             (UINT64)Surface);

    return Surface;
}
#endif
