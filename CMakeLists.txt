cmake_minimum_required(VERSION 3.12)

# List of commandline options
option(ENABLE_LTO "Enable link time optimization" ON)
option(ENABLE_DOCTESTS "Include tests in the library" ON)
option(ENABLE_DOCUMENTATION "Build documentation" ON)

# require c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# project name
project(stepslicerengine
    VERSION 0.0.0
    DESCRIPTION "Slicer Engine"
    LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(ENABLE_DOCTESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# generate version info
configure_file(
    "${PROJECT_SOURCE_DIR}/include/sse/version.hpp.in"
    "${PROJECT_BINARY_DIR}/include/sse/version.hpp"
    @ONLY
)

set(SOURCES
    src/Importer.cpp include/sse/Importer.hpp
    src/pathgenerator.cpp include/sse/pathgenerator.hpp
    src/slicer.cpp include/sse/slicer.hpp
    src/Slice.cpp include/sse/Slice.hpp
    src/Object.cpp include/sse/Object.hpp
    include/sse/version.hpp
)

# main library
add_library(libsse OBJECT ${SOURCES})

# use binary dir for private includes, because of generated files
target_include_directories(libsse
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# search for OpenCASCADE
# TODO: specify OCCT version
find_package(OpenCASCADE REQUIRED)
message(STATUS "OpenCASCADE v${OpenCASCADE_VERSION} found")
include_directories(${OpenCASCADE_INCLUDE_DIR})

# TODO: trim to only used libraries
set(OpenCASCADE_USED_LIBS ${OpenCASCADE_LIBRARIES})

# link library dependencies
target_link_libraries(libsse
    PUBLIC
        stdc++fs
        ${OpenCASCADE_LIBRARIES}
#        clipper
    PRIVATE
)

# Set up executable
add_executable(sse app/main.cpp)
target_compile_features(sse PUBLIC cxx_std_17)
target_link_libraries(sse PRIVATE libsse)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/)

install(TARGETS libsse
    EXPORT libsse-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT libsse-targets
    FILE
        libsseTargets.cmake
    NAMESPACE
        libsse::
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/libsse
)

# Create ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sseConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/sseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sseConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Findsse.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Findsse.cmake
    COPYONLY)

# install config, configversion, and find modules
install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmake/Findsse.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/sseConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/sseConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)



export(PACKAGE StepSlicerEngine)
