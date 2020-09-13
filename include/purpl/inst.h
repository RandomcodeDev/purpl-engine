#pragma once

#ifndef PURPL_INST_H
#define PURPL_INST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

#include "macro.h"
#include "types.h"

#include "log.h"
#include "util.h"
#include "window.h"

namespace purpl {
class engine_info {
public:
	window win;
};
};

#endif /* !PURPL_INST_H */
