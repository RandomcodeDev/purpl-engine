/// @file main.c
///
/// @brief This file implements the cross-platform entry point.
///
/// @copyright (c) 2024 Randomcode Developers

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/packfile.h"

#include "engine/engine.h"

// #define PURPL_TESTING_IN_MAIN

INT PurplMain(_In_ PCHAR *Arguments, _In_ UINT ArgumentCount)
{
#ifdef PURPL_TESTING_IN_MAIN
    PPACKFILE Pack = PackLoad("test");
    UINT64 Size = 0;
    PVOID Data = PackReadFile(Pack, "test.bin", 0, 0, &Size, 0);

    return 0;
#else
    EngDefineVariables();
    CmnInitialize(Arguments, ArgumentCount);
    EngInitialize();

    ecs_entity_t CameraEntity = EcsCreateEntity("camera");
    ecs_add(EcsGetWorld(), CameraEntity, CAMERA);
    CAMERA Camera;
    DOUBLE Aspect = (DOUBLE)RdrGetWidth() / (DOUBLE)RdrGetHeight();
    EngInitializePerspectiveCamera((vec3){0.0, 2.0, 2.0}, (vec4){1.0, 0.0, 0.0, 45.0}, 78.0, Aspect, 0.1, 1000.0,
                                   &Camera);
    ecs_set_ptr(EcsGetWorld(), CameraEntity, CAMERA, &Camera);
    EngSetMainCamera(CameraEntity);

    ecs_entity_t TestEntity = EcsCreateEntity("test");
    PMESH TestMesh = LoadMesh(EngGetAssetPath(EngAssetDirectoryModels, "chief.pmdl"));
    MATERIAL TestMaterial = {0};
    RdrCreateMaterial(&TestMaterial, (RENDER_HANDLE)1, "main");
    MODEL TestModel = {0};
    RdrCreateModel(&TestModel, TestMesh, &TestMaterial);
    ecs_add(EcsGetWorld(), TestEntity, MODEL);
    ecs_set_ptr(EcsGetWorld(), TestEntity, MODEL, &TestModel);

    ecs_add(EcsGetWorld(), TestEntity, TRANSFORM);
    ecs_set(EcsGetWorld(), TestEntity, TRANSFORM, {{0.0, 0.0, 0.0}, {0.0, 1.0, 0.0, -180.0}, {1.0, 1.0, 1.0}});

    ecs_entity_t GroundEntity = EcsCreateEntity("ground");
    PMESH GroundMesh = LoadMesh(EngGetAssetPath(EngAssetDirectoryModels, "ground.pmdl"));
    MATERIAL GroundMaterial = {0};
    RdrCreateMaterial(&GroundMaterial, (RENDER_HANDLE)1, "main");
    MODEL GroundModel = {0};
    RdrCreateModel(&GroundModel, GroundMesh, &GroundMaterial);
    ecs_add(EcsGetWorld(), GroundEntity, MODEL);
    ecs_set_ptr(EcsGetWorld(), GroundEntity, MODEL, &GroundModel);

    ecs_add(EcsGetWorld(), GroundEntity, TRANSFORM);
    ecs_set(EcsGetWorld(), GroundEntity, TRANSFORM, {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0, 90.0}, {1.0, 1.0, 1.0}});

    CONFIGVAR_SET_INT("rdr_clear_colour", 0x000000FF);

    EngMainLoop();

    RdrDestroyModel(&TestModel);
    RdrDestroyMaterial(&TestMaterial);
    CmnFree(TestMesh);

    RdrDestroyModel(&GroundModel);
    RdrDestroyMaterial(&GroundMaterial);
    CmnFree(GroundMesh);

    EngShutdown();
    CmnShutdown();

    return 0;
#endif
}
