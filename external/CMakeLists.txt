# search for OpenMP
find_package(OpenMP)

# add toml11, don't build tests
set(toml11_BUILD_TEST OFF CACHE STRING "Omit toml tests" FORCE)
add_subdirectory(toml11 EXCLUDE_FROM_ALL)

# add spdlog
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

# add cxxopts
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE STRING "Omit cxxopts examples")
set(CXXOPTS_BUILD_TESTS OFF CACHE STRING "Omit cxxopts tests")
add_subdirectory(cxxopts)
