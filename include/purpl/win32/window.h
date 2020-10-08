#pragma once

#ifndef PURPL_WIN32_WINDOW_H
#define PURPL_WIN32_WINDOW_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "macro.h"

#include <windows.h>
#include <winuser.h>

#include "purpl/log.h"
#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
class P_EXPORT win32_window {
    public:
	HWND handle; /* The native handle for the actual window. Only use this for platform specific code. */
	MSG win_queue; /* The queue of events for the window. Only use this for platform specific code. */
	bool should_close; /* Whether the window should close. Set to true manually to close the window. */

	/*
	 * The window procedure.
	 * Defined in window.cc
	 */
	static LRESULT CALLBACK wndproc(HWND window, uint msg, WPARAM wparam,
					LPARAM lparam);

	/* 
	 * This creates a window.
	 * Defined in window.cc
	 */
	win32_window(int width = 1024, int height = 600, bool keep_console = false, const char *title = "Purpl Engine", ...);

	/*
	 * Updates the window.
	 * Pass NULL for any of the parameters to leave it unchanged/use the default.
	 * Defined in window.cc
	 */
	void update(int width = NULL, int height = NULL, const char *title = NULL, ...);

	/* Frees the resources for this window.
	 * Defined in window.cc
	 * Not needed yet.
	 *
	~win32_window(void); */

    private:
	int width;
	int height;
	char *title;
};
}

#endif /* !PURPL_WIN32_WINDOW_H */
