# ---- Variables ----

# We use variables separate from what CTest uses, because those have
# customization issues
set(
    COVERAGE_TRACE_COMMAND
    lcov -c -q
    -o "${PROJECT_BINARY_DIR}/coverage.info"
    -d "${PROJECT_BINARY_DIR}"
    # --ignore-errors inconsistent
    # --filter range  # TODO - currently need these flags to get working locally
    --include "${PROJECT_SOURCE_DIR}/*"
    CACHE STRING
    "; separated command to generate a trace for the 'coverage' target"
)

set(
    COVERAGE_HTML_COMMAND
    genhtml --legend -q
    # --ignore-errors inconsistent
    # --ignore-errors range
    # --ignore-errors category
    "${PROJECT_BINARY_DIR}/coverage.info"
    -p "${PROJECT_SOURCE_DIR}"
    -o "${PROJECT_BINARY_DIR}/coverage_html"
    CACHE STRING
    "; separated command to generate an HTML report for the 'coverage' target"
)

# ---- Coverage target ----

add_custom_target(
    coverage
    COMMAND ${COVERAGE_TRACE_COMMAND}
    COMMAND ${COVERAGE_HTML_COMMAND}
    COMMENT "Generating coverage report"
    VERBATIM
)
