cmake_minimum_required(VERSION 3.8...3.13)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR})
endif()

# project name
project(stepslicerengine
    VERSION 0.1
    DESCRIPTION "Slicer Engine"
    LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cake")



configure_file(
    "${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${PROJECT_SOURCE_DIR}/include/version.h"
)

# library
add_library(libsse
    src/importer.cpp include/importer.h
    src/pathgenerator.cpp include/pathgenerator.h
    src/slice.cpp include/slice.h
)

target_include_directories(libsse
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(libsse PUBLIC cxx_std_17)
# link filesystem TS
target_link_libraries(libsse stdc++fs)

# add tests subdirectory
add_subdirectory(test)
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/catch)
add_library(Catch2::Catch IMPORTED INTERFACE)
#set_property(Catch2::Catch PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CATCH_INCLUDE_DIR}")


# search for OpenCASCADE
find_package(OpenCASCADE 7.3.0 REQUIRED)
message(STATUS "OpenCASCADE v${OpenCASCADE_VERSION} found")
include_directories(${OpenCASCADE_INCLUDE_DIR})
# link OpenCASCADE libs
target_link_libraries(libsse ${OpenCASCADE_USED_LIBS})
message(STATUS "${OpenCASCADE_USED_LIBS}")

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/)

install(TARGETS libsse
    EXPORT libsse-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(libsse PROPERTIES EXPORT_NAME "")

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT libsse-targets
    FILE
        libsseTargets.cmake
    NAMESPACE
        libsse::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Create ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/StepSlicerEngineConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/StepSlicerEngineConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/StepSlicerEngineConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

export(PACKAGE StepSlicerEngine)

