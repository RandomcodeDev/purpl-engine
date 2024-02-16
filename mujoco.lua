add_requires("cereal", "glm", "spdlog")

target("ccd")
    set_kind("static")
    add_headerfiles("deps/ccd/*.h", "deps/external/ccd/src/*.h", "deps/external/ccd/src/ccd/*.h")
    add_files("deps/external/ccd/src/*.c")
    add_includedirs("deps/external/ccd/src")

    on_load(fix_target)

target("qhull")
    set_kind("static")
    add_headerfiles("deps/external/qhull/src/libqhull_r/*.h")
    add_files("deps/external/qhull/src/libqhull_r/*.c")

    on_load(fix_target)

target("sdflib")
    set_kind("static")
    add_headerfiles("deps/external/sdflib/include/SdfLib/*.h", "deps/external/sdflib/src/sdf/*.h")
    add_files("deps/external/sdflib/src/sdf/*.cpp")
    add_includedirs("deps/external/sdflib/include", "deps/external/sdflib/libs/InteractiveComputerGraphics", "deps/external/sdflib/src")
    add_packages("cereal", "glm", "spdlog")

    on_load(fix_target)

target("mujoco")
    set_kind("static")
    add_headerfiles("deps/mujoco/include/mujoco/*.h", "deps/mujoco/src/engine/*.h", "mujoco.lua")
    add_files("deps/mujoco/src/engine/*.c", "deps/mujoco/src/engine/*.cc")
    add_includedirs("deps/external/ccd/src", "deps/external/qhull/src/libqhull_r", "deps/external/sdflib/include", "deps/mujoco/src")
    add_deps("ccd", "qhull", "sdflib")
    add_defines("MJ_STATIC=1")

    on_load(fix_target)
