# executable
add_executable(sse main.cpp)

target_compile_features(sse PUBLIC cxx_std_17)
target_link_libraries(sse PRIVATE libsse)


