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
#include "log.h"
#include "util.h"
#include "window.h"

namespace purpl {
class P_EXPORT engine_inst {
public:
	app_info *info;
	window *wnd;

	/* 
	 * Basically to correctly do this you just have
	 * to pass new <class name>(<custom args>) for each class argument.
	 * If you want the defaults pass pass new <class name>().
	 * Defined in inst.cc
	 */
	engine_inst(app_info *info = new app_info(), window *win = new window(), bool write_hello = true);
};
};

#endif /* !PURPL_INST_H */
