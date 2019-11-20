include(CTest)

add_custom_target(check
    COMMAND echo [----] Running tests
    USES_TERMINAL
    COMMAND $${CMAKE_CTEST_COMMAND} --output-on-failure -C Debug
    DEPENDS libsse unittests)

add_custom_target(checkVerbose
    COMMAND echo [----] Running tests
    USES_TERMINAL
    COMMAND $${CMAKE_CTEST_COMMAND} -V --output-on-failure -C Debug
    DEPENDS libsse unittests)

add_test(NAME "runnable"
    COMMAND $<TARGET_FILE:sse>)

add_test(NAME "doctests_unit_tests"
    COMMAND $<TARGET_FILE:unittests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(unittests EXCLUDE_FROM_ALL
    main.cpp
)

target_compile_definitions(unittests PRIVATE UNIT_TESTS)
target_link_libraries(unittests PRIVATE doctest::doctest)

