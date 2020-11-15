# list of all test names
set(TEST_NAMES
  test_main.cpp
  test_binpack.cpp
  test_object.cpp
)


add_executable(unit_test ${TEST_NAMES})

target_compile_features(unit_test PRIVATE cxx_std_17)

target_link_libraries(unit_test
  PRIVATE
    doctest::doctest
    libsse::libsse
)

add_test(NAME UnitTests COMMAND unit_test)
