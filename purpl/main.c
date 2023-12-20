/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    main.c

Abstract:

    This module implements the cross platform entry point.

--*/

#include "purpl/purpl.h"

#include "common/common.h"

#include "engine/engine.h"
#include "engine/entity.h"

#include "platform/platform.h"

INT
PurplMain(
    _In_ INT ArgumentCount,
    _In_ PCHAR* Arguments
    )
/*++

Routine Description:

    This routine is the entry point after the real entry point.
    It orchestrates everything else, including engine initialization,
    the main loop, and shutdown.

Arguments:

    ArgumentCount - The number of arguments.

    Arguments - List of arguments.

Return Value:

    An appropriate status code.

--*/
{
    BOOLEAN Running;

    UNREFERENCED_PARAMETER(ArgumentCount);
    UNREFERENCED_PARAMETER(Arguments);

    PlatformInitialize();
    CommonInitialize();
    EngineInitialize();

    PURPL_ASSERT(RenderLoadFont("purpl-regular"));

    PURPL_ASSERT(RenderLoadShader(
        "basic",
        ShaderTypeMesh
        ));
    PURPL_ASSERT(RenderLoadTexture("chief"));
    RenderAddMaterial(
        "chief",
        "basic",
        "chief"
        );
    PURPL_ASSERT(RenderLoadModel("chief", 0));

    ecs_entity_t Camera = EngineCreateEntity("camera");
    ecs_add(
        EngineGetEcsWorld(),
        Camera,
        CAMERA
        );
    CAMERA CameraComponent;
    InitializePerspectiveCamera(
        (vec3){0.0, 0.0, 2.0},
        (vec4){0.0, 0.0, 0.0, 0.0},
        78.0,
        1,
        0.1,
        1000.0,
        &CameraComponent
        );
    ecs_set_id(
        EngineGetEcsWorld(),
        Camera,
        ecs_id(CAMERA),
        sizeof(CAMERA),
        &CameraComponent
        );
    EngineSetMainCamera(Camera);

    ecs_entity_t MasterChief = EngineCreateEntity("chief");
    ecs_add(
        EngineGetEcsWorld(),
        MasterChief,
        TRANSFORM
        );
    ecs_add(
        EngineGetEcsWorld(),
        MasterChief,
        RENDERABLE
        );
    ecs_set(
        EngineGetEcsWorld(),
        MasterChief,
        RENDERABLE,
        {RenderableTypeModel, RenderGetModel("chief")}
        );

    ecs_entity_t MasterChief2 = EngineCreateEntity("chief2");
    ecs_add(
        EngineGetEcsWorld(),
        MasterChief,
        TRANSFORM
        );
    ecs_add(
        EngineGetEcsWorld(),
        MasterChief,
        RENDERABLE
        );
    ecs_set(
        EngineGetEcsWorld(),
        MasterChief,
        RENDERABLE,
        {RenderableTypeModel, RenderGetModel("chief")}
        );

    Running = TRUE;
    FLOAT Rotation = 90.0;
    while ( Running )
    {
        Running = PlatformUpdate();

        Rotation += 20.0f * (FLOAT)EngineGetDelta();
        if ( Rotation > 360.0f )
        {
            Rotation = 0.0f;
        }

        ecs_set(
            EngineGetEcsWorld(),
            MasterChief,
            TRANSFORM,
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, Rotation}, {1.0f, 1.0f, 1.0f}}
            );
        ecs_set(
            EngineGetEcsWorld(),
            MasterChief2,
            TRANSFORM,
            {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, Rotation}, {1.0f, 1.0f, 1.0f}}
            );
        EngineUpdate();
    }

    RenderDestroyModel("chief");
    RenderDestroyTexture("chief");
    RenderDestroyShader("basic");
    RenderDestroyFont("purpl-regular");

    EngineShutdown();
    CommonShutdown();
    PlatformShutdown();

    return 0;
}
