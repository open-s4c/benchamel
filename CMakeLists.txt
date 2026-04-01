cmake_minimum_required(VERSION 3.16)
project(benchamel LANGUAGES C CXX)

set(BENCHAMEL_SYSTEM_NAME
    "${CMAKE_SYSTEM_NAME}"
    CACHE STRING "Benchamel target system name")

option(BENCHAMEL_BUILD_TSAN "Enable ThreadSanitizer" ON)

if(${BENCHAMEL_BUILD_TSAN})
  add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer)
  link_libraries(-fsanitize=thread)
endif()

set(BENCHAMEL_DISTFILES
    # "${CMAKE_BINARY_DIR}/_distfiles"
    "${CMAKE_SOURCE_DIR}/_distfiles"
    CACHE PATH "Directory for downloaded distfile archives")

option(BENCHAMEL_DISTFILES_IN_SOURCE
       "Extract distfiles inside the source tree instead of the build tree" ON)

set(BENCHAMEL_DISTFILES_BASE_URL
    "https://github.com/open-s4c/benchamel/releases/download/distfiles/"
    CACHE STRING
          "Base URL checked before each benchmark's upstream distfile URL")

include(cmake/distfiles.cmake)
include(cmake/utils.cmake)
benchamel_log_config()

# add_subdirectory(convul) add_subdirectory(libvsync) add_subdirectory(sctbench)
# add_subdirectory(buf_ring)
add_subdirectory(concurrency-bugs)
add_subdirectory(period/pbzip2-1.1.13)

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
    get_target_property(external_exec_path ${target} BENCHAMEL_EXECUTABLE_PATH)
    if(type STREQUAL "EXECUTABLE")
      set(output ${output} ${target})
    elseif(external_exec_path)
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
  get_target_property(external_exec_path ${exec} BENCHAMEL_EXECUTABLE_PATH)
  get_target_property(external_exec_name ${exec} BENCHAMEL_EXECUTABLE_NAME)
  if(external_exec_path)
    set(EXECUTABLES_ ${EXECUTABLES_}
                     "${external_exec_name}:${external_exec_path}")
  else()
    set(EXECUTABLES_
        ${EXECUTABLES_}
        "$<TARGET_FILE_PREFIX:${exec}>$<TARGET_FILE_BASE_NAME:${exec}>:$<TARGET_FILE:${exec}>"
    )
  endif()
endforeach()

# dump list into a file
string(REPLACE ";" "\n" EXECUTABLES "${EXECUTABLES_}")
file(
  GENERATE
  OUTPUT "bugs.lst"
  CONTENT ${EXECUTABLES})
