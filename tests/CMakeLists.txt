project(sse_tests)

# The test program
add_executable(tests main.cpp)
target_link_libraries(tests PRIVATE libsse catch)
target_compile_definitions(tests PRIVATE CATCH_CONFIG_CONSOLE_WIDTH=60)
