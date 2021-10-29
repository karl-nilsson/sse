add_executable(unit_test)

target_sources(
    unit_test
    PRIVATE
        test_main.cpp
        test_rearrange.cpp
        test_object.cpp
        test_settings.cpp
        test_slice.cpp
)

target_compile_features(unit_test PRIVATE cxx_std_17)

target_link_libraries(unit_test PRIVATE doctest::doctest libsse::libsse)

# copy the files necessary to run the tests
add_custom_command(
    TARGET unit_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS
        -E
        copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${PROJECT_BINARY_DIR}/tests/resources
)

add_test(NAME UnitTests COMMAND unit_test)
