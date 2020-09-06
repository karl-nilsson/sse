project(sse)

# Set up executable
add_executable(${PROJECT_NAME} main.cpp)
target_compile_features(sse PUBLIC cxx_std_17)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        libsse::libsse
        cxxopts
)

# install location
install(TARGETS sse
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT sse_Runtime
)
