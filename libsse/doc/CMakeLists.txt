find_package(Doxygen COMPONENTS dot)
if(DOXYGEN_FOUND)
    # set doxygen options
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    set(DOXYGEN_ENABLE_PREPROCESSING YES)
    # graph options
    set(DOXYGEN_CLASS_DIAGRAMS YES)
    set(DOXYGEN_CALLER_GRAPH YES)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 5)
    set(DOXYGEN_DOT_NUM_THREADS 4)
    set(DOXYGEN_UML_LOOK YES)
    set(DOXYGEN_COLLABORATION_GRAPH YES)
    # include private sources
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_REFERENCED_BY_RELATION YES)
    set(DOXYGEN_REFERENCES_RELATION YES)
    # this function automatically sets Doxyfile's project name, version, and description
    doxygen_add_docs(
        doc
        # sources
        ${PROJECT_SOURCE_DIR}
        # include generated headers
        ${PROJECT_BINARY_DIR}/include
        # add to default build target
        ALL
        # target comment
        COMMENT "Generating documentation"
    )

    # install documentation
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html DESTINATION doc)
endif()
