# Set up executable
add_executable(sse main.cpp)
target_compile_features(sse PUBLIC cxx_std_17)

# find_package(libsse REQUIRED)

target_include_directories(sse
    PRIVATE
        ${PROJECT_SOURCE_DIR}/extern/cxxopts
)

message(STATUS "dir ${PROJECT_SOURCE_DIR}")

# link to engine library
target_link_libraries(sse
    PRIVATE
        libsse::libsse
)

# install location
install(TARGETS sse
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
