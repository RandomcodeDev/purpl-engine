#pragma once

#ifndef PURPL_X11_WINDOW_H
#define PURPL_X11_WINDOW_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <X11/Xlib.h>

#include "purpl/log.h"
#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl{
class P_EXPORT x11_window {
public:
	Window handle; /* The native handle for the actual window. Only use this for platfrom specific code */
	XEvent win_queue; /* The queue of events for the window. Only use this for platform specific code */
	bool should_close; /* Whether the window should close. Set to true manually to close the window. */
	int width;
	int height;
	char *title;

	/*
	 * This creates the window.
	 * Defined in window.cc
	 */
	x11_window(int width = 1024, int height = 600, const char *title = "Purpl Engine", ...);

	/*
	 * Updates the window.
	 * Pass NULL for any of the parameters to leave it unchanged/use the default.
	 * Defined in window.cc
	 */
	void update(int width = NULL, int height = NULL, const char *title = NULL, ...);

	/*
	 * Frees the resources for this window.
	 * Defined in window.cc
	 * Not needed yet.
	 *
	~x11_window(void); */
};
}

#endif
