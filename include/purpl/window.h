#pragma once

#ifndef PURPL_WINDOW_H
#define PURPL_WINDOW_H

#ifdef _WIN32
#include "win32/window.h"

typedef win32_window window;
#else
#error "Window creation is unsupported on this platform."
#endif

#endif /* !PURPL_WINDOW_H */
