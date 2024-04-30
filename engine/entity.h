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

#define ECS_SYSTEM_DEFINE_EX(world, id_, phase, multi_threaded_, interval_, ...)                                       \
    {                                                                                                                  \
        ecs_system_desc_t desc = {0};                                                                                  \
        ecs_entity_desc_t edesc = {0};                                                                                 \
        edesc.id = ecs_id(id_);                                                                                        \
        edesc.name = #id_;                                                                                             \
        edesc.add[0] = ((phase) ? ecs_pair(EcsDependsOn, (phase)) : 0);                                                \
        edesc.add[1] = (phase);                                                                                        \
        desc.entity = ecs_entity_init(world, &edesc);                                                                  \
        desc.query.filter.expr = #__VA_ARGS__;                                                                         \
        desc.callback = id_;                                                                                           \
        desc.multi_threaded = (multi_threaded_);                                                                       \
        desc.interval = (interval_);                                                                                   \
        ecs_id(id_) = ecs_system_init(world, &desc);                                                                   \
    }                                                                                                                  \
    ecs_assert(ecs_id(id_) != 0, ECS_INVALID_PARAMETER, NULL)

#define ECS_SYSTEM_EX(world, id, phase, multi_threaded, interval, ...)                                                 \
    ecs_entity_t ecs_id(id) = 0;                                                                                       \
    ECS_SYSTEM_DEFINE_EX(world, id, phase, multi_threaded, interval, __VA_ARGS__);                                     \
    ecs_entity_t id = ecs_id(id);                                                                                      \
    (void)ecs_id(id);                                                                                                  \
    (void)id

/// @brief Define configuration variables
extern VOID EcsDefineVariables(VOID);

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
