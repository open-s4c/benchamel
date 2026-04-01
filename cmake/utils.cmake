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
# always JCB.<target>.
function(register_bug_target target executable_path)
  set_target_properties(
    "${target}" PROPERTIES BENCHAMEL_EXECUTABLE_PATH "${executable_path}"
                           BENCHAMEL_EXECUTABLE_NAME "JCB.${target}")
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
