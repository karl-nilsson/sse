cmake_minimum_required(VERSION 3.16)

# project basics
project(
    stepslicerengine
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
option(BUILD_DOC "Build library documentation" OFF)
option(CODE_COVERAGE "Enable coverage reporting" OFF)

# global settings
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# add custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

# hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# get git revision information
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_describe(GIT_TAG)

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
            set_target_properties(
                ${PROJECT_NAME}
                PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
            )
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

# search for OpenCASCADE
# n.b. OpenCASCADConfigVersion.cmake compatibility = ExactVersion
# therefore, version range won't work
find_package(OpenCASCADE REQUIRED)
message(STATUS "OpenCASCADE v${OpenCASCADE_VERSION} found")
if(OpenCASCADE_VERSION VERSION_LESS "7.3.0")
    message(FATAL_ERROR "OpenCASCADE version mismatch, expected >= 7.3.0")
endif()

# list of all OCCT libs: OpenCASCADE_LIBRARIES
# TODO: trim to only used libs
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
    # "TKRWMesh"
    # "TKTopTest"
    # "TKDCAF"
)

# add external dependencies
add_subdirectory(external)

# add library
add_subdirectory(libsse)

# add executable
add_subdirectory(app)

add_library(coverage_config INTERFACE)
# code coverage switch
if(CODE_COVERAGE)
    target_compile_options(
        coverage_config
        INTERFACE
            -O0 # disable optimizations
            -g # add debug info
            --coverage # set coverage flags
    )

    target_link_options(coverage_config INTERFACE --coverage)
endif()

# tests switch
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    # enable code coverage
    # add tests
    add_subdirectory(tests)
endif()
