cmake_minimum_required(VERSION 3.14)

foreach(var IN ITEMS PROJECT_BINARY_DIR PROJECT_SOURCE_DIR)
  if(NOT DEFINED "${var}")
    message(FATAL_ERROR "${var} must be defined")
  endif()
endforeach()
set(bin "${PROJECT_BINARY_DIR}")
set(src "${PROJECT_SOURCE_DIR}")

# ---- Process project() call in CMakeLists.txt ----

file(READ "${src}/CMakeLists.txt" content)

string(FIND "${content}" "project(" index)
if(index EQUAL "-1")
  message(FATAL_ERROR "Could not find \"project(\"")
endif()
string(SUBSTRING "${content}" "${index}" -1 content)

string(FIND "${content}" "\n)\n" index)
if(index EQUAL "-1")
  message(FATAL_ERROR "Could not find \"\\n)\\n\"")
endif()
string(SUBSTRING "${content}" 0 "${index}" content)

file(WRITE "${bin}/docs-ci.project.cmake" "docs_${content}\n)\n")

macro(list_pop_front list out)
  list(GET "${list}" 0 "${out}")
  list(REMOVE_AT "${list}" 0)
endmacro()

function(docs_project name)
  cmake_parse_arguments(PARSE_ARGV 1 "" "" "VERSION;DESCRIPTION;HOMEPAGE_URL" LANGUAGES)
  set(PROJECT_NAME "${name}" PARENT_SCOPE)
  if(DEFINED _VERSION)
    set(PROJECT_VERSION "${_VERSION}" PARENT_SCOPE)
    string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" versions "${_VERSION}")
    string(REPLACE . ";" versions "${versions}")
    set(suffixes MAJOR MINOR PATCH TWEAK)
    while(NOT versions STREQUAL "" AND NOT suffixes STREQUAL "")
      list_pop_front(versions version)
      list_pop_front(suffixes suffix)
      set("PROJECT_VERSION_${suffix}" "${version}" PARENT_SCOPE)
    endwhile()
  endif()
  if(DEFINED _DESCRIPTION)
    set(PROJECT_DESCRIPTION "${_DESCRIPTION}" PARENT_SCOPE)
  endif()
  if(DEFINED _HOMEPAGE_URL)
    set(PROJECT_HOMEPAGE_URL "${_HOMEPAGE_URL}" PARENT_SCOPE)
  endif()
endfunction()

include("${bin}/docs-ci.project.cmake")

# ---- Generate docs ----

if(NOT DEFINED DOXYGEN_OUTPUT_DIRECTORY)
  set(DOXYGEN_OUTPUT_DIRECTORY "${bin}/docs")
endif()
set(out "${DOXYGEN_OUTPUT_DIRECTORY}")

foreach(file IN ITEMS Doxyfile conf.py)
  configure_file("${src}/docs/${file}.in" "${bin}/docs/${file}" @ONLY)
endforeach()

set(config "${bin}/docs/conf.py")

file(REMOVE_RECURSE "${out}/html" "${out}/xml")

# Add option to fail on warnings
option(FAIL_ON_WARNINGS "Fail the build if there are warnings" OFF)

execute_process(
    COMMAND doxygen "${bin}/docs/Doxyfile"
    COMMENT "Building documentation using Doxygen"
    VERBATIM
    RESULT_VARIABLE result
)

message(STATUS "Doxygen exit status: ${result}")

if(FAIL_ON_WARNINGS AND NOT result EQUAL 0)
  message(FATAL_ERROR "Doxygen build failed with result ${result}")
endif()