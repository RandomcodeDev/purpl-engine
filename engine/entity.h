/// @file entity.h
///
/// @brief This file defines ECS components and helper functions for entities.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/configvar.h"

#include "util/mesh.h"
#include "util/texture.h"

/// @brief Initialize the ECS
extern VOID EcsInitialize(VOID);

/// @brief Begin an ECS frame
///
/// @param[in] Delta The amount of time since this was last called in milliseconds
extern VOID EcsBeginFrame(_In_ UINT64 Delta);

/// @brief End an ECS frame
extern VOID EcsEndFrame(VOID);

/// @brief Shut down the ECS
extern VOID EcsShutdown(VOID);

/// @brief Create an entity
///
/// @param[in] Name The name of the entity
///
/// @return An entity
extern ecs_entity_t EcsCreateEntity(_In_opt_ PCSTR Name);

/// @brief Set the ECS world
///
/// @param[in] World The new world to use
extern VOID EcsSetWorld(_In_ ecs_world_t *World);

/// @brief Get the ECS world
///
/// @return The current world
extern ecs_world_t *EcsGetWorld(VOID);
