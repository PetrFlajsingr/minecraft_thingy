cmake_minimum_required(VERSION 3.20)
include("cmake/HunterGate.cmake")

set(CPM_DOWNLOAD_LOCATION "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
set(CPM_DOWNLOAD_VERSION 0.32.2)

message(https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake)
if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION} AND CPM_VERSION STREQUAL CPM_DOWNLOAD_VERSION))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})


HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.269.tar.gz"
        SHA1 "64024b7b95b4c86d50ae05b926814448c93a70a0"
)


project(minecraft_thingy)


set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(GLFW_INCLUDE_NONE)
add_compile_definitions(GLM_FORCE_SWIZZLE)
add_compile_definitions(FMT_HEADER_ONLY )

find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(src)
include_directories(include/stb)

find_package(GLFW3 REQUIRED)

CPMAddPackage(
        NAME pf_common
        GITHUB_REPOSITORY PetrFlajsingr/pf_common
        GIT_TAG v1.1.1b
)

CPMAddPackage(
        NAME pf_imgui
        GITHUB_REPOSITORY PetrFlajsingr/pf_imgui
        GIT_TAG v6.0.2b
)


#add_compile_definitions(SPDLOG_FMT_EXTERNAL)
hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(glad src/glad.c)

file(GLOB GEGL_SOURCES src/geGL/*.cpp src/geGL/private/*.cpp)

add_executable(minecraft_thingy
        src/main.cpp
        src/ui/Window.cpp
        src/ui/Window.h
        src/utils/files.h
        src/utils/files.cpp
        ${pf_imgui_SOURCE_DIR}/src/pf_imgui/backends/ImGuiGlfwOpenGLInterface.cpp
        ${pf_imgui_SOURCE_DIR}/src/pf_imgui/backends/impl/imgui_impl_glfw.cpp
        ${pf_imgui_SOURCE_DIR}/src/pf_imgui/backends/impl/imgui_impl_opengl3.cpp
        src/renderers/Renderer.h src/geGL/Shader.cpp src/geGL/Shader.h
        ${GEGL_SOURCES} include/Camera.h src/renderers/MinecraftThingyRenderer.cpp
        src/renderers/MinecraftThingyRenderer.h src/world/Chunk.cpp src/world/Chunk.h
        src/world/Voxel.h src/ui/UI.cpp src/ui/UI.h src/noise/NoiseGenerator.h
        src/noise/PerlinNoiseGenerator.h src/noise/PerlinNoiseGenerator.cpp src/log.h
        src/utils/Direction.h
        src/utils/FPSCounter.cpp
        src/world/ChunkManager.cpp src/world/ChunkManager.h src/utils/Random.h src/utils/LowResPoint.h)


target_link_libraries(minecraft_thingy glad glfw ${OPENGL_LIBRARIES} pf_common::pf_common pf_imgui::pf_imgui nlohmann_json::nlohmann_json #fmt::fmt spdlog::spdlog
        )

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/dlls"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
## disabled, because this erases any data saved by the UI
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        "${PROJECT_SOURCE_DIR}/config.toml"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>/config.toml)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/src/shaders"
        "${PROJECT_SOURCE_DIR}/resources/shaders")


#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()