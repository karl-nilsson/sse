cmake_minimum_required(VERSION 3.15)

# project basics
project(stepslicerengine
    VERSION 0.0.0
    DESCRIPTION "Slicer Engine"
    LANGUAGES CXX
)

# enable testing
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()


# List of commandline options
option(ENABLE_LTO "Enable link time optimization" OFF)

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

if(FALSE)
if(ENABLE_LTO)
    # Check if we can enable Link Time Optimization
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
    if(supported)
        message(STATUS "IPO / LTO supported, enabling")
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>, disabled")
    endif()
endif()
endif()

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

# build library
add_subdirectory(libsse)

# add cxxopts
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE STRING "Omit cxxopts examples")
add_subdirectory(external/cxxopts)

# add executable
add_subdirectory(app/)

# tests switch
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    # disable doctests' internal tests
    set(DOCTEST_WITH_TESTS OFF CACHE STRING "Omit doctest tests" FORCE)
    # include doctest
    add_subdirectory(external/doctest EXCLUDE_FROM_ALL)
    # add tests
    add_subdirectory(tests)
endif()




