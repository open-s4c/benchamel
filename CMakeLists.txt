cmake_minimum_required(VERSION 3.16)
project(benchamel LANGUAGES C CXX)

message("Building on ${CMAKE_SYSTEM_NAME}")

option(BENCHAMEL_BUILD_TSAN "Enable ThreadSanitizer" OFF)

add_subdirectory(convul)
add_subdirectory(libvsync)
add_subdirectory(sctbench)
add_subdirectory(buf_ring)
add_subdirectory(concurrency-bugs)

# ------------------------------------------------------------------------------
# generate bugs.lst file with the list of all executables that are buggy
# ------------------------------------------------------------------------------
function(get_executables DIR OUTPUT)
  set(output)

  get_property(
    TARGETS
    DIRECTORY "${DIR}"
    PROPERTY BUILDSYSTEM_TARGETS)

  foreach(target IN LISTS TARGETS)
    get_target_property(type ${target} TYPE)
    if(type STREQUAL "EXECUTABLE")
      set(output ${output} ${target})
    endif()
  endforeach()

  get_property(
    SUBDIRS
    DIRECTORY "${DIR}"
    PROPERTY SUBDIRECTORIES)
  foreach(subdir IN LISTS SUBDIRS)
    set(_output)
    get_executables("${subdir}" _output)
    set(output ${output} ${_output})
  endforeach()

  set(${OUTPUT}
      ${output}
      PARENT_SCOPE)
endfunction()

set(EXECUTABLES)
get_executables(. EXECUTABLES)

# remove all executables with _ok and _unsat suffix as these cases do not
# represent a buggy binary
list(FILTER EXECUTABLES EXCLUDE REGEX "^.*_ok$")
list(FILTER EXECUTABLES EXCLUDE REGEX "^.*_unsat$")

# append binary path to each target
set(EXECUTABLES_)
foreach(exec IN LISTS EXECUTABLES)
  set(EXECUTABLES_ ${EXECUTABLES_} "${exec}:$<TARGET_FILE:${exec}>")
endforeach()

# dump list into a file
string(REPLACE ";" "\n" EXECUTABLES "${EXECUTABLES_}")
file(
  GENERATE
  OUTPUT "bugs.lst"
  CONTENT ${EXECUTABLES})
