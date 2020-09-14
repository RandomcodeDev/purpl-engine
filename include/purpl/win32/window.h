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

#include "../log.h"
#include "../macro.h"
#include "../types.h"
#include "../util.h"
#include "../../src/win32/window.cc"

#define WINDOW_TEXT_MAX 90

namespace purpl
{
class __declspec(dllexport) window {
    public:
	HWND handle;
	MSG win_queue;
	bool should_close;

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
	window(int width, int height, const wchar_t *title, bool keep_console);

	/*
	 * Updates the window.
	 * Pass NULL for any of the parameters to leave it unchanged/use the default.
	 * Defined in window.cc
	 */
	void update(int width, int height, const wchar_t *title);

	/* Closes the window.
	 * Defined in window.cc
	 * Not needed yet.
	~window(); */

    private:
	int width;
	int height;
	wchar_t title[WINDOW_TEXT_MAX];
};
}

#endif /* !PURPL_WIN#@_WINDOW_H */
