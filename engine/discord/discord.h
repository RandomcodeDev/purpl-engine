/// @file discord.h
/// @file discord.h
///
/// @brief This file defines functions for interfacing with Discord
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/alloc.h"
#include "common/common.h"
#include "common/filesystem.h"
#include "common/log.h"

#include "platform/platform.h"

/// @brief Cooldown (in milliseconds) between API calls
#define DISCORD_API_COOLDOWN 1000 / 50

/// @brief Initialize Discord RPC
extern VOID DiscordInitialize(VOID);

/// @brief Update Discord RPC
extern VOID DiscordUpdate(VOID);

/// @brief Shut down Discord RPC
extern VOID DiscordShutdown(VOID);

/// @brief Whether Discord is connected
extern BOOLEAN DiscordConnected(VOID);
