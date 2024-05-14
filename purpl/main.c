/// @file main.c
///
/// @brief This file implements the cross-platform entry point.
///
/// @copyright (c) 2024 Randomcode Developers

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/packfile.h"

#include "engine/engine.h"

VOID Spin(_In_ ecs_iter_t *Iterator)
{
    PROTATION Rotation = ecs_field(Iterator, ROTATION, 2);

    for (UINT32 i = 0; i < Iterator->count; i++)
    {
        // Only on Y
        if (Rotation[i].Value[1] >= 1.0)
        {
            Rotation[i].Value[3] += 30 * Iterator->delta_time;
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

    ECS_SYSTEM(EcsGetWorld(), Spin, EcsOnUpdate, MODEL, ROTATION);

    ecs_entity_t CameraEntity = EcsCreateEntity("camera");
    CamAddPerspective(CameraEntity, CONFIGVAR_GET_FLOAT("cam_fov"), FALSE, 0.1, 1000.0);
    ecs_set(EcsGetWorld(), CameraEntity, POSITION, {{0.0, 2.0, 2.0}});
    ecs_set(EcsGetWorld(), CameraEntity, ROTATION, {{1.0, 0.0, 0.0, 45.0}});
    EngSetMainCamera(CameraEntity);

    ecs_entity_t TestEntity = EcsCreateEntity("test");
    RENDER_HANDLE TestTexture = RdrLoadTexture("chief.ptex");
    MATERIAL TestMaterial = {0};
    RdrCreateMaterial(&TestMaterial, TestTexture, "main");
    MODEL TestModel = {0};
    RdrLoadModel(&TestModel, "chief.pmdl", &TestMaterial);
    ecs_set_ptr(EcsGetWorld(), TestEntity, MODEL, &TestModel);
    RENDER_OBJECT_DATA TestObject = {0};
    RdrInitializeObject("test", &TestObject, &TestModel);
    ecs_set_ptr(EcsGetWorld(), TestEntity, RENDER_OBJECT_DATA, &TestObject);
    ecs_set(EcsGetWorld(), TestEntity, POSITION, {{-1.0, 0.0, 0.0}});
    ecs_set(EcsGetWorld(), TestEntity, ROTATION, {{0.0, 1.0, 0.0, 270.0}});
    ecs_set(EcsGetWorld(), TestEntity, SCALE, {{1.0, 1.0, 1.0}});

    ecs_entity_t Test2Entity = EcsCreateEntity("test2");
    RENDER_HANDLE Test2Texture = RdrLoadTexture("chief_alt.ptex");
    MATERIAL Test2Material = {0};
    RdrCreateMaterial(&Test2Material, Test2Texture, "main");
    MODEL Test2Model = {0};
    RdrLoadModel(&Test2Model, "chief.pmdl", &Test2Material);
    ecs_set_ptr(EcsGetWorld(), Test2Entity, MODEL, &Test2Model);
    RENDER_OBJECT_DATA Test2Object = {0};
    RdrInitializeObject("test2", &Test2Object, &Test2Model);
    ecs_set_ptr(EcsGetWorld(), Test2Entity, RENDER_OBJECT_DATA, &Test2Object);
    ecs_set(EcsGetWorld(), Test2Entity, POSITION, {{1.0, 0.0, 0.0}});
    ecs_set(EcsGetWorld(), Test2Entity, ROTATION, {{0.0, 1.0, 0.0, -270.0}});
    ecs_set(EcsGetWorld(), Test2Entity, SCALE, {{1.0, 1.0, 1.0}});

    ecs_entity_t GroundEntity = EcsCreateEntity("ground");
    RENDER_HANDLE GroundTexture = RdrLoadTexture("ground.ptex");
    MATERIAL GroundMaterial = {0};
    RdrCreateMaterial(&GroundMaterial, GroundTexture, "main");
    MODEL GroundModel = {0};
    RdrLoadModel(&GroundModel, "ground.pmdl", &GroundMaterial);
    ecs_set_ptr(EcsGetWorld(), GroundEntity, MODEL, &GroundModel);
    RENDER_OBJECT_DATA GroundObject = {0};
    RdrInitializeObject("ground", &GroundObject, &GroundModel);
    ecs_set_ptr(EcsGetWorld(), GroundEntity, RENDER_OBJECT_DATA, &GroundObject);

    ecs_set(EcsGetWorld(), GroundEntity, POSITION, {{0.0, -0.5, 0.0}});
    ecs_set(EcsGetWorld(), GroundEntity, ROTATION, {{1.0, 0.0, 0.0, 90.0}});
    ecs_set(EcsGetWorld(), GroundEntity, SCALE, {{1.0, 1.0, 1.0}});

    CONFIGVAR_SET_INT("rdr_clear_colour", 0x000000FF);

    EngMainLoop();

    RdrDestroyObject(ecs_get(EcsGetWorld(), GroundEntity, RENDER_OBJECT_DATA));
    RdrDestroyModel(ecs_get(EcsGetWorld(), GroundEntity, MODEL));
    RdrDestroyMaterial(&GroundMaterial);

    RdrDestroyObject(ecs_get(EcsGetWorld(), Test2Entity, RENDER_OBJECT_DATA));
    RdrDestroyModel(ecs_get(EcsGetWorld(), Test2Entity, MODEL));
    RdrDestroyMaterial(&Test2Material);

    RdrDestroyObject(ecs_get(EcsGetWorld(), TestEntity, RENDER_OBJECT_DATA));
    RdrDestroyModel(ecs_get(EcsGetWorld(), TestEntity, MODEL));
    RdrDestroyMaterial(&TestMaterial);

    RdrDestroyTexture(GroundTexture);
    RdrDestroyTexture(Test2Texture);
    RdrDestroyTexture(TestTexture);

    EngShutdown();
    CmnShutdown();

    return 0;
#endif
}
