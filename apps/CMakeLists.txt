# executable
add_executable(sse src/main.cpp)
target_include_directories(sse PRIVATE include)

target_compile_features(sse PUBLIC cxx_std_17)
