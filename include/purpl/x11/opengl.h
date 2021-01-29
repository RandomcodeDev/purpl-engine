#pragma once

#ifndef PURPL_X11_OPENGL_H
#define PURPL_X11_OPENGL_H 1

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "purpl/macro.h"

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "purpl/log.h"
#include "purpl/types.h"
#include "purpl/window.h"

namespace purpl {
class P_EXPORT x11_opengl_inst {
    public:
	/* Used to indicate whether the instance is alive */
	bool is_active;

	/
};
}

#endif /* !PURPL_X11_OPENGL_H */
