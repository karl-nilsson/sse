project(libsse)

# generate version info
configure_file(
    "${PROJECT_SOURCE_DIR}/include/sse/version.hpp.in"
    "${PROJECT_BINARY_DIR}/include/sse/version.hpp"
    @ONLY
)

# main library
add_library(${PROJECT_NAME} SHARED)
# create alias libsse::libsse
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# prevent "liblibsse.so"
set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "sse"
  SOVERSION 0
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

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
      include/sse/Packer.hpp
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
        coverage_config           # interface library used to generate coverage data
        stdc++fs                  # filesystem
        ${OpenCASCADE_USED_LIBS}  # opencascade
#        clipper
        toml11::toml11
        spdlog::spdlog_header_only
    PRIVATE
        project_options
# Generates too many warnings for external libs
#        project_warnings
)

# build documentation
if(BUILD_DOC)
  add_subdirectory(doc)
endif()

#FIXME
if(FALSE)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# install targets
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT ${PROJECT_NAME}_Runtime
      NAMELINK_COMPONENT ${PROJECT_NAME}_Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT ${PROJECT_NAME}_Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT ${PROJECT_NAME}_Runtime
)

# Create libsseConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    # maybe ${PROJECT_VERSION} ???
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
)

# generate libsseConfig.cmake
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION
      ${INSTALL_CONFIGDIR}
)

# export targets to a script
install(
    EXPORT ${PROJECT_NAME}_Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# install config and configversion
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# install headers
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


# ????
# exporting from build tree
export(
    EXPORT ${PROJECT_NAME}_Targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
)

export(PACKAGE ${PROJECT_NAME})

endif()
#FIXME
