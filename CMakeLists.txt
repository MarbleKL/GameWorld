cmake_minimum_required(VERSION 3.20)
project(GameWorld)

set(CMAKE_CXX_STANDARD 20)

# 确保编译器使用 UTF-8 编码
if(MSVC)
    add_compile_options(/utf-8)
elseif(CMAKE_COMPILER_IS_GNUXX OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # MinGW/GCC: 设置输入和输出编码为 UTF-8
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

# ====================================
# 自动发现源文件（带构建时依赖检测）
# ====================================

# CONFIGURE_DEPENDS: 每次构建时自动检测文件增删
# 优点：新增/删除文件无需手动修改 CMakeLists.txt
# 注意：CMake 3.12+ 支持

file(GLOB SYSTEMS_SOURCES
    CONFIGURE_DEPENDS
    "src/systems/*.cpp"
)

file(GLOB ECS_SOURCES
    CONFIGURE_DEPENDS
    "src/ecs/*.cpp"
)

file(GLOB PROCESS_SOURCES
    CONFIGURE_DEPENDS
    "src/process/*.cpp"
)

file(GLOB SIMULATION_SOURCES
    CONFIGURE_DEPENDS
    "src/simulation/*.cpp"
)

file(GLOB EXPORT_SOURCES
    CONFIGURE_DEPENDS
    "src/export/*.cpp"
)

file(GLOB GEOMETRY_SOURCES
    CONFIGURE_DEPENDS
    "src/geometry/*.cpp"
)

file(GLOB TOOLS_SOURCES
    CONFIGURE_DEPENDS
    "src/tools/*.cpp"
)

file(GLOB RENDER_SOURCES
    CONFIGURE_DEPENDS
    "src/render/*.cpp"
)

file(GLOB VECTOR2D_SOURCES
    CONFIGURE_DEPENDS
    "src/vector2d/*.cpp"
)

# 主程序
set(MAIN_SOURCES
    main.cpp
)

# 合并所有源文件
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${SYSTEMS_SOURCES}
    ${ECS_SOURCES}
    ${PROCESS_SOURCES}
    ${SIMULATION_SOURCES}
    ${EXPORT_SOURCES}
    ${GEOMETRY_SOURCES}
    ${TOOLS_SOURCES}
    ${RENDER_SOURCES}
    ${VECTOR2D_SOURCES}
)

# 调试输出：显示发现的文件
message(STATUS "Found ${CMAKE_PROJECT_NAME} sources:")
message(STATUS "  Systems:    ${SYSTEMS_SOURCES}")
message(STATUS "  ECS:        ${ECS_SOURCES}")
message(STATUS "  Process:    ${PROCESS_SOURCES}")
message(STATUS "  Simulation: ${SIMULATION_SOURCES}")
message(STATUS "  Export:     ${EXPORT_SOURCES}")
message(STATUS "  Geometry:   ${GEOMETRY_SOURCES}")
message(STATUS "  Tools:      ${TOOLS_SOURCES}")
message(STATUS "  Render:     ${RENDER_SOURCES}")
message(STATUS "  Vector2D:   ${VECTOR2D_SOURCES}")
message(STATUS "  Main:       ${MAIN_SOURCES}")

add_executable(GameWorld ${ALL_SOURCES})

# ====================================
# GDExtension 集成（可选编译）
# ====================================

option(BUILD_GDEXTENSION "Build Godot GDExtension library" ON)

if(BUILD_GDEXTENSION)
    message(STATUS "Building GDExtension for Godot...")

    # 下载 godot-cpp (GDExtension SDK)
    include(FetchContent)
    FetchContent_Declare(
        godot_cpp
        GIT_REPOSITORY https://github.com/godotengine/godot-cpp
        GIT_TAG godot-4.5-stable
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(godot_cpp)

    # GDExtension 源文件
    file(GLOB GDEXTENSION_SOURCES
        CONFIGURE_DEPENDS
        "godot-extension/*.cpp"
    )

    # GDExtension 共享库
    add_library(gameworld_gdextension SHARED
        ${GDEXTENSION_SOURCES}
        ${SYSTEMS_SOURCES}
        ${ECS_SOURCES}
        ${PROCESS_SOURCES}
        ${SIMULATION_SOURCES}
        # 不包含 main.cpp 和 export/tools
    )

    target_link_libraries(gameworld_gdextension PRIVATE godot::cpp)
    target_include_directories(gameworld_gdextension PRIVATE ${CMAKE_SOURCE_DIR}/src)

    # 输出到 Godot 插件目录
    set_target_properties(gameworld_gdextension PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/godot-game-world/addons/gameworld_simulation/bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/godot-game-world/addons/gameworld_simulation/bin"
        PREFIX ""  # 移除 lib 前缀 (Windows)
    )

    message(STATUS "GDExtension will be output to: godot-game-world/addons/gameworld_simulation/bin/")
endif()
