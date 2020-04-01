cmake_minimum_required(VERSION 3.15)

# project basics
project(stepslicerengine
    VERSION 0.0.0
    DESCRIPTION "Slicer Engine"
    LANGUAGES CXX
)



# List of commandline options
option(ENABLE_LTO "Enable link time optimization" OFF)
option(ENABLE_TESTING "Build tests" ON)

# require c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# global settings
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# add custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(NOT APPLE)
  set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

# fake library for options
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# enable sanitizers
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# static analysis
include(cmake/StaticAnalyzers.cmake)


# enable ccache
find_program(CCACHE ccache)
if(CCACHE)
  message(STATUS "ccache enabled")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
  message(STATUS "ccache not found, disabled")
endif()

# enable documentation
include(cmake/Doxygen.cmake)
enable_doxygen()



# search for OpenCASCADE
# TODO: specify OCCT version
find_package(OpenCASCADE REQUIRED)
message(STATUS "OpenCASCADE v${OpenCASCADE_VERSION} found")
# list of all OCCT libs: OpenCASCADE_LIBRARIES
# trim to only used libs
set(OpenCASCADE_USED_LIBS
  "TKernel"
  "TKMath"
  "TKG2d"
  "TKG3d"
  "TKGeomBase"
  "TKBRep"
  "TKGeomAlgo"
  "TKTopAlgo"
  "TKPrim"
  "TKBO"
# "TKShHealing"
  "TKBool"
  "TKHLR"
  "TKOffset"
  "TKFeat"
  "TKMesh"
  "TKXMesh"
  "TKCDF"
# "TKLCAF"
# "TKCAF"
  "TKBinL"
  "TKXmlL"
  "TKBin"
  "TKXml"
  "TKStdL"
  "TKStd"
  "TKTObj"
  "TKBinTObj"
# "TKXmlTObj"
# "TKVCAF"
  "TKXSBase"
  "TKSTEPBase"
  "TKSTEPAttr"
  "TKSTEP209"
  "TKSTEP"
  "TKIGES"
# "TKXCAF"
  "TKXDEIGES"
  "TKXDESTEP"
  "TKSTL"
# "TKXmlXCAF"
# "TKBinXCAF"
  "TKRWMesh"
# "TKTopTest"
# "TKDCAF"
  )


# search for OpenMP
find_package(OpenMP)

# add toml11, don't build tests
set(toml11_BUILD_TEST OFF CACHE STRING "Omit toml tests" FORCE)
add_subdirectory(external/toml11 EXCLUDE_FROM_ALL)

# add spdlog
add_subdirectory(external/spdlog EXCLUDE_FROM_ALL)

# generate version info
configure_file(
    "${PROJECT_SOURCE_DIR}/include/sse/version.hpp.in"
    "${PROJECT_BINARY_DIR}/include/sse/version.hpp"
    @ONLY
)

# main library
add_library(libsse SHARED)
# alias
add_library(libsse::libsse ALIAS libsse)
# prevent "liblibsse.so"
set_target_properties(libsse PROPERTIES
  OUTPUT_NAME "sse"
  SOVERSION 0
)

# TODO: correctly sort files into proper context
target_sources(libsse
  PRIVATE
      src/Importer.cpp
      src/slicer.cpp
      src/Slice.cpp
      src/Object.cpp
      src/Settings.cpp
      src/Support.cpp
      src/Packer.cpp
      src/Packer.hpp
  PUBLIC
      include/sse/Importer.hpp
      include/sse/slicer.hpp
      include/sse/Slice.hpp
      include/sse/Object.hpp
      include/sse/Settings.hpp
      include/sse/Support.hpp
  INTERFACE
)


target_include_directories(libsse BEFORE
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # generated headers found here
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        ${OpenCASCADE_INCLUDE_DIR}
        # FIXME: this needs to be private
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}/src

)


# link library dependencies
target_link_libraries(libsse
    PUBLIC
        stdc++fs
        ${OpenCASCADE_USED_LIBS}
        OpenMP::OpenMP_CXX
#        clipper
        toml11::toml11
    PRIVATE
        spdlog::spdlog_header_only
        project_options
# Generates too many warnings for external libs
#        project_warnings
)


if(ENABLE_LTO)
    ### Check if we can enable Link Time Optimization
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
    if(supported)
        message(STATUS "IPO / LTO supported, enabling")
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>, disabled")
    endif()
endif()



#FIXME
if(FALSE)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/libsse)

# install targets
install(
    TARGETS libsse
    EXPORT libsse-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT libsse_Runtime
      NAMELINK_COMPONENT libsse_Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT libsse_Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT libsse_Runtime
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
    COMPATIBILITY SameMajorVersion
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

endif()
#FIXME

# tests switch
if(ENABLE_TESTING)
    # disable doctests' internal tests
    set(DOCTEST_WITH_TESTS OFF CACHE STRING "Omit doctest tests" FORCE)
    add_subdirectory(external/doctest EXCLUDE_FROM_ALL)
    enable_testing()
    add_subdirectory(tests)
endif()



add_subdirectory(external/cxxopts)
# add executable
add_subdirectory(app/)
