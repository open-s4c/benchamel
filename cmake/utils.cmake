include_guard(GLOBAL)

# copy_asset(<source-rel-path>)
#
# Copies an asset from the current source directory into the current binary
# directory, preserving only the basename of the source path.
function(copy_asset source_rel_path)
  set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/${source_rel_path}")
  get_filename_component(asset_name "${source_rel_path}" NAME)
  configure_file("${source_path}" "${CMAKE_CURRENT_BINARY_DIR}/${asset_name}"
                 COPYONLY)
endfunction()

# register_bug_target(<target> <built-binary-path>)
#
# Registers a non-CMake-built benchmark executable, exports it to
# ${CMAKE_BINARY_DIR}/bin as <BENCHMARK_NAME>.<target>, and records that final
# path so the top-level bugs.lst generator can treat the benchmark like a normal
# executable target.
function(register_bug_target target built_binary_path)
  if(DEFINED BENCHMARK_NAME)
    set(prefix "${BENCHMARK_NAME}")
  else()
    set(prefix "benchamel")
  endif()
  set(exported_path "${CMAKE_BINARY_DIR}/bin/${prefix}.${target}")
  set(export_target "${target}-export")

  add_custom_command(
    OUTPUT "${exported_path}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/bin"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${built_binary_path}"
            "${exported_path}"
    DEPENDS "${built_binary_path}"
    VERBATIM)
  add_custom_target("${export_target}" DEPENDS "${exported_path}")
  add_dependencies("${target}" "${export_target}")

  set_target_properties(
    "${target}" PROPERTIES BENCHAMEL_EXECUTABLE_PATH "${exported_path}"
                           BENCHAMEL_EXECUTABLE_NAME "${prefix}.${target}")
endfunction()

# configure_bug_target(<target>)
#
# Sets the standard runtime output location and exposed <BENCHMARK_NAME>. prefix
# for a normal CMake-built benchmark target, defaulting to JCB. when no
# BENCHMARK_NAME is set.
function(configure_bug_target target)
  if(DEFINED BENCHMARK_NAME)
    set(prefix "${BENCHMARK_NAME}.")
  else()
    set(prefix "benchamel.")
  endif()
  set_target_properties(
    "${target}" PROPERTIES PREFIX "${prefix}" RUNTIME_OUTPUT_DIRECTORY
                                              "${CMAKE_BINARY_DIR}/bin")
endfunction()

# benchamel_log_config()
#
# Prints all CMake configurations related of Benchamel.
function(benchamel_log_config)
  get_cmake_property(cache_variables CACHE_VARIABLES)
  list(SORT cache_variables)

  message(STATUS "Benchamel configuration:")
  foreach(cache_var IN LISTS cache_variables)
    if(cache_var MATCHES "^BENCHAMEL_")
      get_property(
        cache_help
        CACHE "${cache_var}"
        PROPERTY HELPSTRING)
      if(cache_help)
        message(STATUS "  ${cache_var}=${${cache_var}} (${cache_help})")
      else()
        message(STATUS "  ${cache_var}=${${cache_var}}")
      endif()
    endif()
  endforeach()
endfunction()

# add_benchmark(<dirname> [<prefix>])
#
# Sets BENCHMARK_NAME while adding a benchmark-suite subdirectory. If <prefix>
# is provided, BENCHMARK_NAME is set to that value for the duration of
# add_subdirectory(<dirname>). Otherwise it defaults to <dirname>. Any previous
# BENCHMARK_NAME value is restored afterwards.
macro(add_benchmark dirname)
  if(DEFINED BENCHMARK_NAME)
    set(_benchamel_saved_benchmark_name "${BENCHMARK_NAME}")
    set(_benchamel_had_benchmark_name TRUE)
  else()
    unset(_benchamel_saved_benchmark_name)
    unset(_benchamel_had_benchmark_name)
  endif()

  if(NOT "${ARGV1}" STREQUAL "")
    set(BENCHMARK_NAME "${ARGV1}")
  else()
    set(BENCHMARK_NAME "${dirname}")
  endif()

  add_subdirectory("${dirname}")

  if(_benchamel_had_benchmark_name)
    set(BENCHMARK_NAME "${_benchamel_saved_benchmark_name}")
  else()
    unset(BENCHMARK_NAME)
  endif()

  unset(_benchamel_saved_benchmark_name)
  unset(_benchamel_had_benchmark_name)
endmacro()

# benchamel_normalize_identifier(<value> <output-var>)
#
# Converts a value into a safe CMake identifier:
# - uppercases characters
# - replaces non [A-Z0-9_] characters with '_'
# - prefixes '_' when the first character is a digit
function(benchamel_normalize_identifier value output_var)
  string(TOUPPER "${value}" normalized)
  string(REGEX REPLACE "[^A-Z0-9_]" "_" normalized "${normalized}")
  if(normalized MATCHES "^[0-9]")
    set(normalized "_${normalized}")
  endif()
  set(${output_var}
      "${normalized}"
      PARENT_SCOPE)
endfunction()

# add_bug(<bug-dir-name> [DEFAULT <ON|OFF>])
#
# Creates an option named <BENCHMARK_NAME>_<bug-dir-name> (both normalized via
# benchamel_normalize_identifier) and adds the bug subdirectory when enabled.
#
# Example:
#   add_bug(transmission-1.42 DEFAULT OFF)
function(add_bug bug_name)
  if(NOT DEFINED BENCHMARK_NAME)
    message(FATAL_ERROR
            "add_bug(${bug_name}) requires BENCHMARK_NAME to be set first")
  endif()

  cmake_parse_arguments(PARSE_ARGV 1 ADD_BUG "" "DEFAULT" "")
  if(ADD_BUG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
            "add_bug(${bug_name}) received unexpected arguments: ${ADD_BUG_UNPARSED_ARGUMENTS}"
    )
  endif()

  if(NOT "${ADD_BUG_DEFAULT}" STREQUAL "")
    string(TOUPPER "${ADD_BUG_DEFAULT}" bug_default)
    if(NOT bug_default STREQUAL "ON" AND NOT bug_default STREQUAL "OFF")
      message(FATAL_ERROR
              "add_bug(${bug_name}) DEFAULT must be ON or OFF, got: ${ADD_BUG_DEFAULT}"
      )
    endif()
  else()
    set(bug_default "ON")
  endif()

  benchamel_normalize_identifier("${BENCHMARK_NAME}" benchmark_norm)
  benchamel_normalize_identifier("${bug_name}" bug_norm)
  set(bug_option "BUG_${benchmark_norm}_${bug_norm}")

  option(${bug_option} "Enable benchmark ${BENCHMARK_NAME}/${bug_name}"
         ${bug_default})

  if(${bug_option})
    add_subdirectory("${bug_name}")
  else()
    message(STATUS
            "Skipping benchmark ${BENCHMARK_NAME}/${bug_name} (${bug_option}=OFF)"
    )
  endif()
endfunction()
