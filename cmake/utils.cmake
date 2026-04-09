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

# register_bug_target(<target> <executable-path>)
#
# Registers a non-CMake-built benchmark executable so the top-level bugs.lst
# generator can treat it like a normal bug target. The exposed benchmark name is
# always <BENCHMARK_NAME>.<target>.
function(register_bug_target target executable_path)
  if(DEFINED BENCHMARK_NAME)
    set(prefix "${BENCHMARK_NAME}")
  else()
    set(prefix "benchamel")
  endif()
  set_target_properties(
    "${target}" PROPERTIES BENCHAMEL_EXECUTABLE_PATH "${executable_path}"
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
    set(prefix "JCB.")
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

  if(ARGC GREATER 1)
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
