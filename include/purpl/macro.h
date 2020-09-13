#pragma once

#ifndef PURPL_MACRO_H
#define PURPL_MACRO_H

#include <string.h>

#include "types.h"

#define P_ARRAYSIZE(arr) (sizeof arr / sizeof arr[0])
#define P_BUFSIZE(buf, type) (strlen(buf) * sizeof(type))

#define P_CONCAT(hi, lo, type, target) ((target)hi << sizeof(type) | lo)

#endif /* !PURPL_MACRO_H */
