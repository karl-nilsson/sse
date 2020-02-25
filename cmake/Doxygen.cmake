function(enable_doxygen)
  option(ENABLE_DOXYGEN "Enable doxygen doc builds of source" OFF)
  if(ENABLE_DOXYGEN)
    message(STATUS "building documentation")
    set(DOXYGEN_CALLER_GRAPH ON)
    set(DOXYGEN_CALL_GRAPH ON)
    set(DOXYGEN_EXTRACT_ALL ON)
    find_package(Doxygen REQUIRED dot)
    # set doxygen options
    set(DOXYGEN_OUTPUT_DIRECTORY doc)
    set(DOXYGEN_COLLABORATION_GRAPH YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_CLASS_DIAGRAMS YES)
    set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 5)
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    # this automatically sets Doxyfile's project name, version, and description
    doxygen_add_docs(
      doc
      ${PROJECT_SOURCE_DIR}/doc
      ${PROJECT_SOURCE_DIR}/include
#      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generate documentation"
    )

  # install documentation
  install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html DESTINATION doc)

  endif()
endfunction()
