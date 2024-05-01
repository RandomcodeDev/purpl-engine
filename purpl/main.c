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
    DOUBLE Aspect = (DOUBLE)RdrGetWidth() / (DOUBLE)RdrGetHeight();
    CamAddPerspective(CameraEntity, 78.0, Aspect, 0.1, 1000.0);
    ecs_set(EcsGetWorld(), CameraEntity, POSITION, {{0.0, 2.0, 2.0}});
    ecs_set(EcsGetWorld(), CameraEntity, ROTATION, {{1.0, 0.0, 0.0, 45.0}});
    EngSetMainCamera(CameraEntity);

    ecs_entity_t TestEntity = EcsCreateEntity("test");
    RENDER_HANDLE TestTexture = RdrLoadTexture("chief.ptex");
    MATERIAL TestMaterial = {0};
    RdrCreateMaterial(&TestMaterial, TestTexture, "main");
    PMODEL TestModel = ecs_emplace(EcsGetWorld(), TestEntity, MODEL);
    RdrLoadModel(TestModel, "chief.pmdl", &TestMaterial);
    PRENDER_OBJECT_DATA TestObject = ecs_emplace(EcsGetWorld(), TestEntity, RENDER_OBJECT_DATA);
    RdrInitializeObject(TestObject);

    ecs_set(EcsGetWorld(), TestEntity, POSITION, {{0.0, 0.0, 0.0}});
    ecs_set(EcsGetWorld(), TestEntity, ROTATION, {{0.0, 1.0, 0.0, -180.0}});
    ecs_set(EcsGetWorld(), TestEntity, SCALE, {{1.0, 1.0, 1.0}});

    ecs_entity_t GroundEntity = EcsCreateEntity("ground");
    MATERIAL GroundMaterial = {0};
    RdrCreateMaterial(&GroundMaterial, TestTexture, "main");
    PMODEL GroundModel = ecs_emplace(EcsGetWorld(), GroundEntity, MODEL);
    RdrLoadModel(GroundModel, "ground.pmdl", &GroundMaterial);
    PRENDER_OBJECT_DATA GroundObject = ecs_emplace(EcsGetWorld(), GroundEntity, RENDER_OBJECT_DATA);
    RdrInitializeObject(GroundObject);

    ecs_set(EcsGetWorld(), GroundEntity, POSITION, {{0.0, -0.5, 0.0}});
    ecs_set(EcsGetWorld(), GroundEntity, ROTATION, {{1.0, 0.0, 0.0, 90.0}});
    ecs_set(EcsGetWorld(), GroundEntity, SCALE, {{1.0, 1.0, 1.0}});

    CONFIGVAR_SET_INT("rdr_clear_colour", 0x000000FF);

    EngMainLoop();

    RdrDestroyObject(TestObject);
    RdrDestroyModel(TestModel);
    RdrDestroyMaterial(&TestMaterial);
    RdrDestroyTexture(TestTexture);

    RdrDestroyObject(GroundObject);
    RdrDestroyModel(GroundModel);
    RdrDestroyMaterial(&GroundMaterial);

    EngShutdown();
    CmnShutdown();

    return 0;
#endif
}
