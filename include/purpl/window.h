#pragma once

#ifndef PURPL_WINDOW_H
#define PURPL_WINDOW_H

#ifdef _WIN32
#include "win32/window.h"

namespace purpl
{
typedef purpl::win32_window window;
}
#elif defined(__linux__)
#include "x11/window.h"

typedef x11_window window;
#else
#error "Window creation is unsupported on this platform."
#endif

#endif /* !PURPL_WINDOW_H */
