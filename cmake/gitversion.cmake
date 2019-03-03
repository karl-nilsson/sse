# if possible, use git to add revision information to the build
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR/.git}")
    message(NOTICE "Git found, generating version info")
    include(GetGitRevisionDescription)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(GIT_SHA ${GIT_SHA} CACHE STRING "git short sha" FORCE)
else()
    message(NOTICE "Git not found, falling back to static version info")
endif()
