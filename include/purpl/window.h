#pragma once

#ifndef PURPL_WINDOW_H
#define PURPL_WINDOW_H

#ifdef _WIN32
#include "win32/window.h"

#define window win32_window
#else
#error "Unsupported platform, sorry"
#endif

#endif /* !PURPL_WINDOW_H */
