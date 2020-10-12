#pragma once

#ifndef PURPL_INST_H
#define PURPL_INST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "macro.h"
#include "types.h"

#include "app_info.h"
#include "graphics.h"
#include "log.h"
#include "util.h"
#include "window.h"

namespace purpl {
class P_EXPORT engine_inst {
public:
	app_info *info; /* The application info for this instance */
	gfx_inst *gfx; /* The graphics instance for this instance */
	window *wnd; /* This instance's window */
	bool is_active; /* Whether the instance is active. */

	/* 
	 * Basically to correctly do this you just have
	 * to pass new <class name>(<custom args>) for each constructor argument.
	 * If you want the defaults pass new <class name>().
	 * Defined in inst.cc
	 */
	engine_inst(app_info *info = new app_info(), gfx_inst *gfx = new gfx_inst(), window *win = new window(), bool write_hello = true);

	/*
	 * Updates the instance (window, graphics, etc.).
	 * Call this in a while loop with is_active as the condition to make your instance loop.
	 * Defined in inst.cc
	 */
	void update(int width = NULL, int height = NULL, const char *title = NULL, ...);

	/*
	 * Cleans up after the instance;
	 * Defined in inst.cc
	 */
	~engine_inst(void);
};
}

#endif /* !PURPL_INST_H */
