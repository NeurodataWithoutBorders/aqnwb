cmake_minimum_required(VERSION 3.14)

macro(default name)
  if(NOT DEFINED "${name}")
    set("${name}" "${ARGN}")
  endif()
endmacro()

default(CPPCHECK_COMMAND cppcheck)
default(SOURCE_DIRS "src;tests")

set(
    cppcheck_args
    --enable=style,performance,portability,warning
    --inline-suppr
    --std=c++17
    --suppress=missingIncludeSystem
    --suppress=duplInheritedMember
    --error-exitcode=1
)

foreach(dir IN LISTS SOURCE_DIRS)
  list(APPEND cppcheck_args "-I${CMAKE_SOURCE_DIR}/${dir}")
endforeach()

set(source_paths "")
foreach(dir IN LISTS SOURCE_DIRS)
  list(APPEND source_paths "${CMAKE_SOURCE_DIR}/${dir}")
endforeach()

execute_process(
    COMMAND "${CPPCHECK_COMMAND}" ${cppcheck_args} ${source_paths}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE result
)

if(NOT result EQUAL "0")
  message(FATAL_ERROR "Cppcheck found issues. See output above.")
endif()
