# attempt to find OpenCasCade
# This module defines the following variables:
# OpenCASCADE_Found
# OpenCASCADE_INCLUDE_DIR
# OpenCASCADE_LIBRARY_DIR
# OpenCASCADE (OpenCASCADE_VERSION_MAJOR, OpenCASCADE_VERSION_MINOR, OpenCASCADE_VERSION_MAINT)


# search for OpenCASCADEConfig.cmake
find_package(OpenCASCADE CONFIG)

# otherwise, manually search
if(NOT OpenCASCADE_FOUND)
    message(STATUS "Could not find OpenCASCADE config, manually searching")

    find_path(OpenCASCADE_CONF "OpenCASCADEConfig.cmake" DOC "Root directory of OpenCASCADE"
        PATHS /usr/lib/cmake/opencascade
    )

    # search for headers
    find_path(OpenCASCADE_INCLUDE_DIR Standard_Version.hxx PATHS
        /usr/include/openCASCADE
        /usr/include/opencascade
        /usr/local/include/opencascade
        /opt/opencascade/include
        /opt/openascade/include
    )
    message(STATUS "Found OpenCASCADE headers: " ${OpenCASCADE_INCLUDE_DIR})
    # find version via headers
    #if(OpenCASCADE_INCLUDE_DIR)
    #    file(STRINGS ${OCC_INCLUDE_DIR}/Standard_Version.hxx OpenCASCADE_VERSION
    #        REGEX "#define OCC_VERSION_MAJOR.*")
    #endif(OpenCASCADE_INCLUDE_DIR)


    find_library(OpenCASCADE_LIBRARY_DIR TKernel PATHS
        /usr/lib
        /usr/local/lib
        /opt/opencascade/lib
    )
    get_filename_component(OpenCASCADE_LIBRARY_DIR ${OpenCASCADE_LIBRARY_DIR} DIRECTORY)
    message(STATUS "Found OpenCASCADE libraries: " ${OpenCASCADE_LIBRARY_DIR})
    # otherwise manually search

endif(NOT OpenCASCADE_FOUND)

# OpenCASCADE_INCLUDE_DIR is wrong, manually override
set(OpenCASCADE_INCLUDE_DIR "/usr/include/opencascade")

#message(STATUS ${OpenCASCADE_INCLUDE_DIR})
#message(STATUS ${OpenCASCADE_LIBRARY_DIR})
#message(STATUS ${OpenCASCADE_MODULES})
#message(STATUS ${OpenCASCADE_INSTALL_PREFIX})

# list of libraries we link to
set (OpenCASCADE_USED_LIBS
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKBRep
    TKGeomAlgo
    TKTopAlgo
    TKPrim
    TKBool
    TKFillet
    TKMesh
    TKOffset
    TKXMesh
    TKIGES
    TKSTEP
    TKSTL
    TKVRML
    TKLCAF
    TKCAF
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCASCADE REQUIRED_VARS OpenCASCADE_INCLUDE_DIR OpenCASCADE_LIBRARY_DIR VERSION_VAR OpenCASCADE_VERSION)


