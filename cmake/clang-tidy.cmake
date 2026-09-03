cmake_minimum_required(VERSION 3.15)

macro(default name)
  if(NOT DEFINED "${name}")
    set("${name}" "${ARGN}")
  endif()
endmacro()

default(CLANG_TIDY_COMMAND clang-tidy)
default(COMPILE_COMMANDS_DIR "${CMAKE_BINARY_DIR}")
default(SOURCE_DIRS "src")

if(NOT EXISTS "${COMPILE_COMMANDS_DIR}/compile_commands.json")
  message(FATAL_ERROR
    "clang-tidy requires a compilation database. Reconfigure with "
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON."
  )
endif()

get_filename_component(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(source_files "")
foreach(dir IN LISTS SOURCE_DIRS)
  file(GLOB_RECURSE dir_source_files "${PROJECT_SOURCE_DIR}/${dir}/*.cpp")
  list(APPEND source_files ${dir_source_files})
endforeach()

if(NOT source_files)
  message(FATAL_ERROR "No C++ source files found for clang-tidy analysis.")
endif()

set(clang_tidy_args
    -p "${COMPILE_COMMANDS_DIR}"
    --warnings-as-errors=*
)

if(APPLE)
  execute_process(
      COMMAND xcrun --show-sdk-path
      OUTPUT_VARIABLE sdk_path
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE sdk_result
  )
  if(NOT sdk_result EQUAL "0")
    message(FATAL_ERROR "Unable to determine the macOS SDK path for clang-tidy.")
  endif()
  list(APPEND clang_tidy_args
      "--extra-arg=-isysroot${sdk_path}"
      "--extra-arg=-isystem${sdk_path}/usr/include/c++/v1"
  )
endif()

execute_process(
    COMMAND "${CLANG_TIDY_COMMAND}" ${clang_tidy_args} ${source_files}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)

if(NOT result EQUAL "0")
  message(FATAL_ERROR
    "clang-tidy failed with result '${result}':\n${output}${error_output}"
  )
endif()
