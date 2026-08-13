find_program(
    default_clang_tidy_command
    NAMES clang-tidy
    HINTS
        /opt/homebrew/opt/llvm/bin
        /usr/local/opt/llvm/bin
)

set(
    CLANG_TIDY_COMMAND
    "${default_clang_tidy_command}"
    CACHE FILEPATH
    "Static analyzer to use"
)

add_custom_target(
    tidy-check
    COMMAND "${CMAKE_COMMAND}"
    -D "CLANG_TIDY_COMMAND=${CLANG_TIDY_COMMAND}"
    -D "COMPILE_COMMANDS_DIR=${PROJECT_BINARY_DIR}"
    -P "${PROJECT_SOURCE_DIR}/cmake/clang-tidy.cmake"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Running clang-tidy static analysis"
    VERBATIM
)
