#pragma once

#ifndef PURPL_WIN32_WINDOW_H
#define PURPL_WIN32_WINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include "macro.h"

#include <windows.h>
#include <winuser.h>

#include <purpl/log.h>
#include <purpl/macro.h>
#include <purpl/types.h>
#include <purpl/util.h>

#define WINDOW_TEXT_MAX 90

namespace purpl
{
class P_EXPORT window {
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
	window(int width, int height, bool keep_console, const wchar_t *title, ...);

	/*
	 * Updates the window.
	 * Pass NULL for any of the parameters to leave it unchanged/use the default.
	 * Defined in window.cc
	 */
	void update(int width, int height, const wchar_t *title, ...);

	/* Frees the resources for this window.
	 * Defined in window.cc
	 * Not needed yet.
	~window(void); */

    private:
	int width;
	int height;
	wchar_t title[WINDOW_TEXT_MAX];
};
}

#endif /* !PURPL_WIN32_WINDOW_H */
