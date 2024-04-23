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

/// @brief Delay in seconds between updates
#define DISCORD_UPDATE_INTERVAL 1.0f

/// @brief Initialize Discord RPC
extern ECS_SYSTEM_DECLARE(DiscordInitialize);

/// @brief Update Discord RPC
extern ECS_SYSTEM_DECLARE(DiscordUpdate);

/// @brief Shut down Discord RPC
extern VOID DiscordShutdown(VOID);

/// @brief Whether Discord is connected
extern BOOLEAN DiscordConnected(VOID);
