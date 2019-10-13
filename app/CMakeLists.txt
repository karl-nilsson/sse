# Set up executable
add_executable(sse main.cpp)
target_compile_features(sse PUBLIC cxx_std_17)
# link to engine library
target_link_libraries(sse PRIVATE libsse::libsse)

# install location
install(TARGETS sse
    DESTINATION bin)
