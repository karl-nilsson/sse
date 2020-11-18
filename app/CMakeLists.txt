project(sse)

# Set up executable
add_executable(${PROJECT_NAME} main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    libsse::libsse
    cxxopts
)

# install location
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT ${PROJECT_NAME}_Runtime
)
