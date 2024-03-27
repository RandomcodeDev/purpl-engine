/// @file ui.h
///
/// @brief This file defines the UI API
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "common/alloc.h"
#include "common/common.h"
#include "common/configvar.h"

#include "platform/video.h"

#include "render/render.h"

/// @brief Initialize the UI
extern VOID UiInitialize(VOID);

/// @brief Begin a UI frame
extern VOID UiBeginFrame(VOID);

/// @brief End a UI frame
extern VOID UiEndFrame(VOID);

/// @brief Shut down the UI
extern VOID UiShutdown(VOID);
