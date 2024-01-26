/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    video.c

Abstract:

    This module abstracts the Unix video functions.

--*/

#include "common/common.h"

#include "platform/platform.h"

#include "GLFW/glfw3.h"

static GLFWwindow* Window;

CHAR WindowTitle[128] = GAME_NAME " v" GAME_VERSION_STRING;
INT WindowWidth;
INT WindowHeight;

BOOLEAN WindowResized;
BOOLEAN WindowFocused;
BOOLEAN WindowClosed;

static
VOID
GlfwErrorCallback(
    _In_ INT Error,
    _In_ PCSTR Description
    )
{
    LogError("GLFW error %d: %s", Error, Description);
}

static
VOID
GlfwResizeCallback(
    _In_ GLFWwindow* ResizedWindow,
    _In_ INT Width,
    _In_ INT Height
    )
{
    if ( ResizedWindow == Window )
    {
        LogInfo("Window resized from %ux%u to %dx%d", WindowWidth, WindowHeight, Width, Height);
        WindowWidth = Width;
        WindowHeight = Height;
        WindowResized = TRUE;
    }
}

static
VOID
GlfwFocusCallback(
    _In_ GLFWwindow* FocusWindow,
    _In_ INT Focused
    )
{
    if ( FocusWindow == Window )
    {
        WindowFocused = (BOOLEAN)Focused;
        LogInfo("Window %s focus", WindowFocused ? "gained" : "lost");
    }
}

VOID
VidInitialize(
    VOID
    )
{
    PCSTR GlfwError;

    LogInfo("Initializing Unix video using GLFW");

    if ( !glfwInit() )
    {
        CmnError("Failed to initialize GLFW: %d %s", glfwGetError(&GlfwError), GlfwError);
    }

    glfwSetErrorCallback(GlfwErrorCallback);

    glfwWindowHint(
        GLFW_CLIENT_API,
        GLFW_NO_API
        );

    WindowWidth = 1280;
    WindowHeight = 720;
    LogInfo("Creating %ux%u window titled " GAME_NAME, WindowWidth, WindowHeight);

    Window = glfwCreateWindow(
        WindowWidth,
        WindowHeight,
        WindowTitle,
        NULL,
        NULL
        );
    if ( !Window )
    {
        CmnError("Failed to create window: %d %s", glfwGetError(&GlfwError), GlfwError);
    }

    glfwSetWindowSizeCallback(
        Window,
        GlfwResizeCallback
        );
    glfwSetWindowFocusCallback(
        Window,
        GlfwFocusCallback
        );

//    glfwShowWindow(Window);
}

BOOLEAN
VidUpdate(
	VOID
	)
/*++

Routine Description:

    This routine processes window events.

Arguments:

    None.

Return Value:

    TRUE - The window is still open.

    FALSE - The window was closed.

--*/
{
    glfwPollEvents();

    WindowClosed = (BOOLEAN)glfwWindowShouldClose(Window);

    glfwGetWindowSize(
        Window,
        &WindowWidth,
        &WindowHeight
        );

    return !WindowClosed;
}

VOID
VidShutdown(
    VOID
    )
/*++

Routine Description:

    This routine cleans up video resources.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogInfo("Shutting down Unix video");

    glfwDestroyWindow(Window);
    glfwTerminate();

    LogInfo("Successfully shut down Unix video");
}

VOID
VidGetSize(
    _Out_opt_ PUINT32 Width,
    _Out_opt_ PUINT32 Height
    )
/*++

Routine Description:

    Stores the width and/or height in the provided parameters.

Arguments:

    Width - A pointer to an integer that recieves the width of the
    window if provided.

    Height - A pointer to an integer that recieves the height of the
    window if provided.

Return Value:

    None.

--*/
{
    Width ? *Width = (UINT32)WindowWidth : 0;
    Height ? *Height = (UINT32)WindowHeight : 0;
}

BOOLEAN
VidResized(
    VOID
    )
/*++

Routine Description:

    Returns whether the window has been resized since the last call.

Arguments:

    None.

Return Value:

    The value of WindowResized.

--*/
{
    BOOLEAN ReturnValue = WindowResized;
    WindowResized = FALSE;
    return ReturnValue;
}

BOOLEAN
VidFocused(
    VOID
    )
/*++

Routine Description:

    Returns whether the window is focused.

Arguments:

    None.

Return Value:

    The value of WindowFocused.

--*/
{
    return WindowFocused;
}

FLOAT
VidGetDpi(
    VOID
    )
{
    FLOAT XDpi;
    FLOAT YDpi;

    XDpi = 0.0f;
    YDpi = 0.0f;
    glfwGetWindowContentScale(
        Window,
        &XDpi,
        &YDpi
        );

    return XDpi;
}

#ifdef PURPL_VULKAN
PVOID
PlatCreateVulkanSurface(
    _In_ PVOID Instance,
    _In_opt_ PVOID WindowHandle
    )
/*++

Routine Description:

    Creates a Vulkan surface from the window.

Arguments:

    Instance - The Vulkan instance to create the surface for.

    WindowHandle - The window to create the surface for. If NULL, the engine's window is used.

Return Value:

    NULL - Surface creation failed.

    Non-NULL - Surface creation successful.

--*/
{
    VkResult Result;
    VkSurfaceKHR Surface;

    LogDebug("Creating Vulkan surface with glfwCreateWindowSurface");

    Result = glfwCreateWindowSurface(
        Instance,
        WindowHandle ? WindowHandle : Window,
        NULL,
        &Surface
        );
    if ( Result != VK_SUCCESS )
    {
        LogError("Failed to create Vulkan surface: VkResult %d", Result);
        return NULL;
    }

    return Surface;
}
#endif
