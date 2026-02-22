set(CPPCHECK_COMMAND cppcheck CACHE STRING "Static analyzer to use")

add_custom_target(
    cppcheck
    COMMAND "${CMAKE_COMMAND}"
    -D "CPPCHECK_COMMAND=${CPPCHECK_COMMAND}"
    -P "${PROJECT_SOURCE_DIR}/cmake/cppcheck.cmake"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Running cppcheck static analysis"
    VERBATIM
)
