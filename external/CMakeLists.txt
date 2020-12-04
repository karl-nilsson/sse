include(FetchContent)

# toml11
FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11
  GIT_TAG v3.7.0
)
# toml11 built options
set(toml11_BUILD_TEST OFF CACHE STRING "Omit toml tests" FORCE)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.9.1
)
# cxxopts
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts
  GIT_TAG v2.2.1
)
# cxxopts build options
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE STRING "Omit cxxopts examples")
set(CXXOPTS_BUILD_TESTS OFF CACHE STRING "Omit cxxopts tests")
set(CXXOPTS_ENABLE_INSTALL OFF CACHE STRING "Disable cxxopts install")

# CavalierContours
FetchContent_Declare(
  CavalierContours
  GIT_REPOSITORY https://github.com/karl-nilsson/CavalierContours
#  GIT_TAG fb7ad8d90fdb
)

# force header-only
set(CAVC_HEADER_ONLY ON)

# add libraries
FetchContent_MakeAvailable(
  toml11
  spdlog
  cxxopts
  CavalierContours
)


# only include doctest if building tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  # doctest
  FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest
    GIT_TAG 2.4.6
  )
  # disable doctests' internal tests
  set(DOCTEST_WITH_TESTS OFF CACHE STRING "Omit doctest tests" FORCE)
  FetchContent_MakeAvailable(doctest)
endif()
