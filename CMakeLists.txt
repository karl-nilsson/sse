cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
# project name
project(stepslicerengine)
# version information
set(stepslicerengine_VERSION_MAJOR 0)
set(stepslicerengine_VERSION_MINOR 1)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")


set(stepslicerengine_VERSION "${stepslicerengine_VERSION_MAJOR}.${stepslicerengine_VERSION_MINOR}")
# add modules


set(SOURCES
    src/main.cpp
    src/importer.cpp
)

add_executable(sse ${SOURCES})


target_compile_features(sse PUBLIC cxx_std_17)
target_include_directories(sse PRIVATE ${PROJECT_SOURCE_DIR}/include)


include_directories(SYSTEM "/usr/include/")


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cMake")

# search for OpenCASCADE
find_package(OpenCASCADE REQUIRED)
message(STATUS "OpenCASCADE v${OpenCASCADE_VERSION} found")
include_directories(${OpenCASCADE_INCLUDE_DIR})
# link OpenCASCADE libs
target_link_libraries(sse ${OpenCASCADE_USED_LIBS})


# find gflags
find_package(gflags REQUIRED)
target_link_libraries(sse gflags)


# add install targets
#install (TARGETS ${PROJECT_NAME} DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/" DESTINATION include)
