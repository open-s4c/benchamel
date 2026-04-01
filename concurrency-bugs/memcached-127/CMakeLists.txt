set(LIBEVENT_PREFIX_DIR "${CMAKE_CURRENT_BINARY_DIR}/libevent")
set(LIBEVENT_BINARY_DIR "${LIBEVENT_PREFIX_DIR}/build")
set(LIBEVENT_ARCHIVE "${LIBEVENT_BINARY_DIR}/lib/libevent.a")
set(LIBEVENT_GENERATED_INCLUDE_DIR "${LIBEVENT_BINARY_DIR}/include")

require_distfile(
  TARGET memcached-127
  NAME memcached-1.4.4
  URL "https://github.com/memcached/memcached/archive/refs/tags/1.4.4.tar.gz"
  HASH "f695c7127364481e63636653f5f07b3b1a75d4c956c99a20e19053353dee3164"
  FILENAME "memcached-1.4.4.tar.gz"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)
require_distfile(
  TARGET memcached-127-libevent
  NAME libevent-2.1.12-stable
  URL
    "https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz"
  HASH "92e6de1be9ec176428fd2367677e61ceffc2ee1cb119035037a27d346b0403bb"
  SOURCE_DIR_VAR LIBEVENT_SOURCE_DIR)

ExternalProject_Add(
  libevent
  SOURCE_DIR "${LIBEVENT_SOURCE_DIR}"
  BINARY_DIR "${LIBEVENT_BINARY_DIR}"
  PREFIX "${LIBEVENT_PREFIX_DIR}"
  TMP_DIR "${LIBEVENT_PREFIX_DIR}/tmp"
  STAMP_DIR "${LIBEVENT_PREFIX_DIR}/stamp"
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
             -DCMAKE_C_FLAGS=${CONCURRENCY_BUGS_C_FLAGS}
             -DCMAKE_CXX_FLAGS=${CONCURRENCY_BUGS_CXX_FLAGS}
             -DCMAKE_EXE_LINKER_FLAGS=${CONCURRENCY_BUGS_LDFLAGS}
             -DCMAKE_SHARED_LINKER_FLAGS=${CONCURRENCY_BUGS_LDFLAGS}
             -DCMAKE_POLICY_VERSION_MINIMUM=3.5
             -DEVENT__LIBRARY_TYPE=STATIC
             -DEVENT__DISABLE_OPENSSL=ON
             -DEVENT__DISABLE_TESTS=ON
             -DEVENT__DISABLE_BENCHMARK=ON
             -DEVENT__DISABLE_SAMPLES=ON
             -DEVENT__DISABLE_REGRESS=ON
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS "${LIBEVENT_ARCHIVE}"
                   "${LIBEVENT_GENERATED_INCLUDE_DIR}/event2/event-config.h")

set(memcached_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/client.c"
    "${SOURCE_DIR}/assoc.c"
    "${SOURCE_DIR}/cache.c"
    "${SOURCE_DIR}/daemon.c"
    "${SOURCE_DIR}/hash.c"
    "${SOURCE_DIR}/items.c"
    "${SOURCE_DIR}/memcached.c"
    "${SOURCE_DIR}/slabs.c"
    "${SOURCE_DIR}/stats.c"
    "${SOURCE_DIR}/thread.c"
    "${SOURCE_DIR}/util.c")

add_executable(memcached-127 ${memcached_sources})
configure_bug_target(memcached-127)
add_dependencies(memcached-127 libevent)
target_include_directories(
  memcached-127
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${SOURCE_DIR}"
          "${LIBEVENT_SOURCE_DIR}/include" "${LIBEVENT_GENERATED_INCLUDE_DIR}")
target_link_directories(memcached-127 PRIVATE "${LIBEVENT_BINARY_DIR}/lib")
target_link_libraries(memcached-127 PRIVATE event pthread m)
target_compile_definitions(
  memcached-127 PRIVATE -DPACKAGE=\"memcached\" -DVERSION=\"1.4.4\"
                        -D_GNU_SOURCE -DIOV_MAX=1024 -DHAVE_SIGIGNORE)
target_compile_options(memcached-127 PRIVATE -Wno-deprecated-declarations)

test_big_endian(MEMCACHED_IS_BIG_ENDIAN)
if(NOT MEMCACHED_IS_BIG_ENDIAN)
  target_compile_definitions(memcached-127 PRIVATE -DENDIAN_LITTLE=1)
endif()

if(APPLE)
  target_compile_definitions(memcached-127 PRIVATE HAVE_HTONLL=1)
endif()
