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

# settings
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# add custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


# search for OpenCASCADE
# TODO: specify OCCT version
find_package(OpenCASCADE REQUIRED)
message(STATUS "OpenCASCADE v${OpenCASCADE_VERSION} found")
# TODO: trim to only used libraries
set(OpenCASCADE_USED_LIBS ${OpenCASCADE_LIBRARIES})

# search for OpenMP
find_package(OpenMP)

# generate version info
configure_file(
    "${PROJECT_SOURCE_DIR}/include/sse/version.hpp.in"
    "${PROJECT_BINARY_DIR}/include/sse/version.hpp"
    @ONLY
    )

# set source files
set(SOURCES
    src/Importer.cpp
    src/pathgenerator.cpp
    src/slicer.cpp
    src/Slice.cpp
    src/Object.cpp
    src/Settings.cpp
#    src/SVGWriter.cpp
)

# main library
add_library(libsse SHARED ${SOURCES})
# alias
add_library(libsse::libsse ALIAS libsse)
# prevent liblibsse.so
set_target_properties(libsse PROPERTIES OUTPUT_NAME "sse")

# use binary dir for includes, because of generated files
target_include_directories(libsse
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # TODO: fix this
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        ${OpenCASCADE_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src

        ${PROJECT_SOURCE_DIR}/extern/spdlog
        ${PROJECT_SOURCE_DIR}/extern/toml11
        ${PROJECT_SOURCE_DIR}/extern/simple-svg
)



# link library dependencies
target_link_libraries(libsse
    PUBLIC
        stdc++fs
        ${OpenCASCADE_LIBRARIES}
        OpenMP::OpenMP_CXX
#        clipper
    PRIVATE
)



include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/libsse)

# targets
install(
    TARGETS libsse
    EXPORT libsse-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# install headers
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# export targets to a script
install(
    EXPORT libsse-targets
    FILE libsseTargets.cmake
    NAMESPACE libsse::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# Create libsseConfigVersion.cmake
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libsseConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# generate libsseConfig.cmake
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/libsseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libsseConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# install config and configversion
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/libsseConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/libsseConfigVersion.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# exporting from build tree
export(
    EXPORT libsse-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/libsseTargets.cmake
    NAMESPACE libsse::
)

export(PACKAGE libsse)


# tests switch
if(ENABLE_DOCTESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# add executable
add_subdirectory(app/)
