# list of all test names
set(TEST_NAMES
  test_main.cpp
  test_binpack.cpp
)


add_executable(unit_test ${TEST_NAMES})
target_link_libraries(unit_test
  PRIVATE
    doctest::doctest
    libsse::libsse
)

add_test(NAME UnitTests COMMAND unit_test)
