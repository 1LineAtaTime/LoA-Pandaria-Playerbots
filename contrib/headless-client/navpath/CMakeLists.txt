# Standalone build of the navpath CLI: only the Detour runtime sources from the repository's
# dep/recastnavigation, no server libraries.
cmake_minimum_required(VERSION 3.16)
project(navpath CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(NAVPATH_RECAST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../dep/recastnavigation"
    CACHE PATH "Path to the recastnavigation sources used by the server")
set(DETOUR_DIR "${NAVPATH_RECAST_DIR}/Detour")
if(NOT EXISTS "${DETOUR_DIR}/Include/DetourNavMesh.h")
  message(FATAL_ERROR "Detour not found at ${DETOUR_DIR}; set NAVPATH_RECAST_DIR")
endif()

add_library(navpath_detour STATIC
  ${DETOUR_DIR}/Source/DetourAlloc.cpp
  ${DETOUR_DIR}/Source/DetourAssert.cpp
  ${DETOUR_DIR}/Source/DetourCommon.cpp
  ${DETOUR_DIR}/Source/DetourNavMesh.cpp
  ${DETOUR_DIR}/Source/DetourNavMeshQuery.cpp
  ${DETOUR_DIR}/Source/DetourNode.cpp
)
target_include_directories(navpath_detour PUBLIC ${DETOUR_DIR}/Include)
target_compile_definitions(navpath_detour PRIVATE NDEBUG)

add_executable(navpath src/navpath.cpp)
target_link_libraries(navpath PRIVATE navpath_detour)

if(MSVC)
  target_compile_options(navpath PRIVATE /W3 /utf-8)
  target_compile_definitions(navpath PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
  target_compile_definitions(navpath_detour PRIVATE _CRT_SECURE_NO_WARNINGS)
  # Static CRT so the exe runs without the VC++ redistributable.
  set_property(TARGET navpath navpath_detour PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
  target_compile_options(navpath PRIVATE -Wall)
endif()
