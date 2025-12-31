cmake_minimum_required(VERSION 3.20)
project(GameWorld)

set(CMAKE_CXX_STANDARD 20)

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

# 主程序
set(MAIN_SOURCES
    main.cpp
)

# 合并所有源文件
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${SYSTEMS_SOURCES}
    ${GEOMETRY_SOURCES}
    ${TOOLS_SOURCES}
    ${RENDER_SOURCES}
)

# 调试输出：显示发现的文件
message(STATUS "Found ${CMAKE_PROJECT_NAME} sources:")
message(STATUS "  Systems:  ${SYSTEMS_SOURCES}")
message(STATUS "  Geometry: ${GEOMETRY_SOURCES}")
message(STATUS "  Tools:    ${TOOLS_SOURCES}")
message(STATUS "  Render:   ${RENDER_SOURCES}")
message(STATUS "  Main:     ${MAIN_SOURCES}")

add_executable(GameWorld ${ALL_SOURCES})
