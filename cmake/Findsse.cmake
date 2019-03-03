#Findsse.cmake
#
# Finds the sse library
#
# This will define the following variables
#
#   sse_FOUND
#   sse_INCLUDE_DIRS
#
# and the following imported targets
#
#    sse:sse
#
# Author:

find_package(PkgConfig)
pkg_check_modules(PC_sse QUIET sse)

find_path(sse_INCLUDE_DIR
    NAMES sse.h
    PATHS ${PC_sse_INCLUDE_DIRS}
    PATH_SUFFIXES sse
)

set(sse_VERSION ${PC_sse_VERSION})

mark_as_advanced(sse_FOUND sse_INCLUDE_DIR sse_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sse
    REQUIRED_VARS sse_INCLUDE_DIR
    VERSION_VAR sse_VERSION
)

if(sse_FOUND)
    get_filename_component(sse_INCLUDE_DIRS ${SSE_INCLUDE_DIR} DIRECTORY)
endif()

if(sse_FOUND AND NOT TARGET sse:sse)
    add_libarry(sse:sse INTERFACE IMPORTED)
    set_TARGET_PROPERTIES(sse:sse PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${sse_INCLUDE_DIRS}"
    )
endif()
