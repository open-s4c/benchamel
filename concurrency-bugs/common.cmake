include(ExternalProject)
include(TestBigEndian)

find_package(BZip2 REQUIRED)
find_package(Threads REQUIRED)

set(CONCURRENCY_BUGS_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(CONCURRENCY_BUGS_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
file(MAKE_DIRECTORY "${CONCURRENCY_BUGS_BIN_DIR}")

set(CONCURRENCY_BUGS_C_FLAGS "${CMAKE_C_FLAGS}")
set(CONCURRENCY_BUGS_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CONCURRENCY_BUGS_LDFLAGS "${CMAKE_EXE_LINKER_FLAGS}")
if(BENCHAMEL_BUILD_TSAN)
  add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer)
  link_libraries(-fsanitize=thread)
  string(APPEND CONCURRENCY_BUGS_C_FLAGS
         " -fsanitize=thread -g -fno-omit-frame-pointer")
  string(APPEND CONCURRENCY_BUGS_CXX_FLAGS
         " -fsanitize=thread -g -fno-omit-frame-pointer")
  string(APPEND CONCURRENCY_BUGS_LDFLAGS " -fsanitize=thread")
endif()

# The original jcbugs tree is now checked into this repository. Keep the
# historical variable name so the per-benchmark CMake files can address the
# local test case directories unchanged.
set(jcbugs_SOURCE_DIR "${CONCURRENCY_BUGS_ROOT_DIR}")

function(configure_bug_target target)
  set_target_properties(
    "${target}"
    PROPERTIES PREFIX "JCB." RUNTIME_OUTPUT_DIRECTORY
                               "${CONCURRENCY_BUGS_BIN_DIR}")
endfunction()
