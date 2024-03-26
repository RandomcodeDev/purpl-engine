/// @file main.c
///
/// @brief This file implements the cross-platform entry point.
///
/// @copyright (c) 2024 Randomcode Developers

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/packfile.h"

#include "engine/engine.h"

VOID ChangeClearColour(_In_ ecs_iter_t *Iterator)
{
    UINT64 ClearColour = CONFIGVAR_GET_INT("rdr_clear_colour");
    UINT8 Red = ((ClearColour >> 24) & 0xFF);
    UINT8 Green = ((ClearColour >> 16) & 0xFF);
    DOUBLE Delta = 100 * EngGetDelta();
    UINT8 Blue = (UINT8)(((ClearColour >> 8) & 0xFF) + Delta) % 255;
    UINT8 Alpha = ((ClearColour >> 0) & 0xFF);
    CONFIGVAR_SET_INT("rdr_clear_colour", Red << 24 | Green << 16 | Blue << 8 | Alpha);
}

VOID Spin(_In_ ecs_iter_t *Iterator)
{
    PTRANSFORM Transform = ecs_field(Iterator, TRANSFORM, 1);

    for (UINT32 i = 0; i < Iterator->count; i++)
    {
        Transform[i].Rotation[3] += EngGetDelta() * 20;
        if (Transform[i].Rotation[3] > 360)
        {
            Transform[i].Rotation[3] = 0;
        }
    }
}

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
    EngInitializePerspectiveCamera((vec3){0.0, 0.0, 2.0}, (vec4){0.0, 0.0, 0.0, 0.0}, 78.0, Aspect, 0.1, 1000.0,
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
    ecs_set(EcsGetWorld(), TestEntity, TRANSFORM, {{0.0, -1.0, 0.0}, {0.0, 1.0, 0.0, -180.0}, {1.0, 1.0, 1.0}});

    CONFIGVAR_SET_INT("rdr_clear_colour", 0x800002FF);
    ECS_SYSTEM(EcsGetWorld(), ChangeClearColour, EcsOnUpdate);
    ECS_SYSTEM(EcsGetWorld(), Spin, EcsOnUpdate, TRANSFORM);

    EngMainLoop();

    RdrDestroyModel(&TestModel);
    RdrDestroyMaterial(&TestMaterial);
    CmnFree(TestMesh);

    EngShutdown();
    CmnShutdown();

    return 0;
#endif
}
