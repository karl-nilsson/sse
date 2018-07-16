cmake_minimum_required(VERSION 3.8)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(stepslicer_engine)

set(stepslicer_engine_VERSION_MAJOR 0)
set(stepslicer_engine_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE "Debug")

add_executable(${PROJECT_NAME} "src/main.cpp")

# require c++17
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(SYSTEM "/usr/include/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cMake")

# search for OpenCASCADE
find_package(OpenCASCADE REQUIRED)
message(STATUS "OpenCASCADE v${OpenCASCADE_VERSION} found")
# OpenCASCADE_INCLUDE_DIR is wrong
include_directories(${OpenCASCADE_INCLUDE_DIR})
# link OpenCASCADE libs
target_link_libraries(${PROJECT_NAME} ${OpenCASCADE_USED_LIBS})

#-----BROKEN-----

# find boost libs
find_package(Boost REQUIRED COMPONENTS program_options)
# link boost libs
foreach(_n ${BOOST_LIBRARIES})
    message(STATUS "library: ${_n}")
endforeach()

target_link_libraries(${PROJECT_NAME} ${BOOST_LIBRARIES})

#-----/BROKEN---


# add install targets
#install (TARGETS ${PROJECT_NAME} DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/" DESTINATION include)
