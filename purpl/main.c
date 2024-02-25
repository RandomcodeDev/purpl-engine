/// @file main.c
///
/// @brief This file implements the cross-platform entry point.
///
/// @copyright (c) 2024 Randomcode Developers

#include "purpl/purpl.h"

#include "common/common.h"

#include "engine/engine.h"

INT PurplMain(_In_ PCHAR *Arguments, _In_ UINT ArgumentCount)
{
    CmnInitialize(Arguments, ArgumentCount);
    EngInitialize();

    ecs_entity_t CameraEntity = EcsCreateEntity("camera");
    ecs_add(EcsGetWorld(), CameraEntity, CAMERA);
    CAMERA Camera;
    DOUBLE Aspect = (DOUBLE)RdrGetWidth() / (DOUBLE)RdrGetHeight();
    InitializePerspectiveCamera((vec3){0.0, 0.0, 2.0}, (vec4){0.0, 0.0, 0.0, 0.0}, 78.0,
                                Aspect, 0.1, 1000.0, &Camera);
    ecs_set_ptr(EcsGetWorld(), CameraEntity, CAMERA, &Camera);
    EngSetMainCamera(CameraEntity);

    ecs_entity_t TestEntity = EcsCreateEntity("test");
    PMESH TestMesh = LoadMesh("assets/models/chief.pmdl");
    MODEL TestModel = {0};
    RdrCreateModel(&TestModel, TestMesh, (PMATERIAL)1);
    ecs_add(EcsGetWorld(), TestEntity, MODEL);
    ecs_set_ptr(EcsGetWorld(), TestEntity, MODEL, &TestModel);

    ecs_add(EcsGetWorld(), TestEntity, TRANSFORM);
    ecs_set(EcsGetWorld(), TestEntity, TRANSFORM, {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}});

    EngMainLoop();

    RdrDestroyModel(&TestModel);
    CmnFree(TestMesh);

    EngShutdown();
    CmnShutdown();

    return 0;
}
