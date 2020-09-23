#pragma once

#ifndef PURPL_WINDOW_H
#define PURPL_WINDOW_H

#ifdef _WIN32
#include "win32/window.h"

#define window win32_window
#else
#warning "Unsupported platform. Graphics will not work for this build."
#endif

#endif /* !PURPL_WINDOW_H */
