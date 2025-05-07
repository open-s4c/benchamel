cmake_minimum_required(VERSION 3.16)

project(
    benchamel
    LANGUAGES C CXX
    VERSION 0.1)

message(INFO "Building on ${CMAKE_SYSTEM_NAME}")

add_subdirectory(ConVul)
add_subdirectory(libvsync)
add_subdirectory(sctbench)

# CMake does not have an easy way how to show the path of all generated
# executables. Here is a recursive function to collect all executable targets.
function(get_executables out root)
  set(out_targets)
  get_directory_property(targets DIRECTORY ${root} BUILDSYSTEM_TARGETS)
  foreach(target IN LISTS targets)
    get_target_property(type ${target} TYPE)
    if(type STREQUAL "EXECUTABLE")
      list(APPEND out_targets ${target})
    endif()
  endforeach()

  get_directory_property(dirs DIRECTORY ${root} SUBDIRECTORIES)
  foreach(dir IN LISTS dirs)
    get_executables(targets ${dir})
    list(APPEND out_targets ${targets})
  endforeach()
  set(${out} ${out_targets} PARENT_SCOPE)
endfunction()

# CMake does not have an easy way how to list the path of the targets, so here
# is a loop to implement that as a target "list-benchmarks"
add_custom_target(list-benchmarks)

get_all_executables(targets ${CMAKE_SOURCE_DIR})
foreach(target IN LISTS targets)
    add_custom_target(print-${target}
	echo "$<TARGET_FILE:${target}>" >> benchmark.list
	COMMAND_EXPAND_LISTS)
    add_dependencies(list-benchmarks print-${target})
endforeach()
