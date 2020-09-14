#pragma once

#ifndef PURPL_WINDOW_H
#define PURPL_WINDOW_H

#ifdef _WIN32
#include "win32/window.h"
#elif defined __APPLE__
#include "../src/MacOS/window.h"
#else
#include "../src/Linux/window.h"
#endif /* _WIN32 */
#endif /* !PURPL_WINDOW_H */
