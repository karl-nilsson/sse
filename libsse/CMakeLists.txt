project(libsse)

# generate version info
configure_file(
    "${PROJECT_SOURCE_DIR}/include/sse/version.hpp.in"
    "${PROJECT_BINARY_DIR}/include/sse/version.hpp"
    @ONLY
)

# main library
add_library(${PROJECT_NAME} SHARED)
# alias
add_library(libsse::libsse ALIAS ${PROJECT_NAME})
# prevent "liblibsse.so"
set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "sse"
  SOVERSION 0
)

# TODO: correctly sort files into proper context
target_sources(${PROJECT_NAME}
  PRIVATE
      src/Importer.cpp
      src/slicer.cpp
      src/Slice.cpp
      src/Object.cpp
      src/Settings.cpp
      src/Support.cpp
      src/Packer.cpp
      include/sse/Importer.hpp
      include/sse/slicer.hpp
      include/sse/Slice.hpp
      include/sse/Object.hpp
      include/sse/Settings.hpp
      include/sse/Support.hpp
      include/Packer.hpp
)

target_include_directories(${PROJECT_NAME} BEFORE
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        # generated headers found here
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        ${OpenCASCADE_INCLUDE_DIR}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)


# link library dependencies
target_link_libraries(${PROJECT_NAME}
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


#FIXME
if(FALSE)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/libsse)

# install targets
install(
    TARGETS ${PROJECT_NAME}
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

export(PACKAGE ${PROJECT_NAME})

endif()
#FIXME
