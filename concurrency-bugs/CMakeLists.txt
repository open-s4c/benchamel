if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

include(CheckCSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(ExternalProject)
include(FetchContent)
include(TestBigEndian)

# fetch Jieyu's concurrency bug collection

# FetchContent_Populate( jcbugs URL
# https://github.com/jieyu/concurrency-bugs/archive/refs/heads/master.zip
# URL_HASH
# SHA256=87d3fe48fe4922f48257a2a77645bd00b74b89bc598043dc95e2cf78762d97b9
# #SOURCE_DIR jieyu )

# add_library(libevent::event STATIC IMPORTED GLOBAL) set_target_properties(
# libevent::event PROPERTIES IMPORTED_LOCATION "${LIBEVENT_ARCHIVE}"
# INTERFACE_INCLUDE_DIRECTORIES
# "${LIBEVENT_SOURCE_DIR}/include;${LIBEVENT_GENERATED_INCLUDE_DIR}")
# add_dependencies(libevent::event libevent_external)

# memcached-127 extract package tar xvfz
# ${CMAKE_CURRENT_BINARY_DIR}/jcbugs-src/memcached-1.4.4.tar.gz

# apply patch cd ${CMAKE_CURRENT_BINARY_DIR}/jcbugs-src patch -p1 <
# ${CMAKE_CURRENT_BINARY_DIR}/jcbugs-src/memcached-1.4.4.tar.gz

set(LIBEVENT_PREFIX_DIR ${CMAKE_CURRENT_BINARY_DIR}/libevent)
set(LIBEVENT_SOURCE_DIR ${LIBEVENT_PREFIX_DIR}/src)
set(LIBEVENT_BINARY_DIR ${LIBEVENT_PREFIX_DIR}/build)
set(LIBEVENT_ARCHIVE ${LIBEVENT_BINARY_DIR}/lib/libevent.a)
set(LIBEVENT_GENERATED_INCLUDE_DIR ${LIBEVENT_BINARY_DIR}/include)

ExternalProject_Add(
  libevent
  URL https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
  URL_HASH
    SHA256=92e6de1be9ec176428fd2367677e61ceffc2ee1cb119035037a27d346b0403bb
  DOWNLOAD_EXTRACT_TIMESTAMP YES
  PREFIX "${LIBEVENT_PREFIX_DIR}"
  SOURCE_DIR "${LIBEVENT_SOURCE_DIR}"
  BINARY_DIR "${LIBEVENT_BINARY_DIR}"
  TMP_DIR "${LIBEVENT_PREFIX_DIR}/tmp"
  STAMP_DIR "${LIBEVENT_PREFIX_DIR}/stamp"
  DOWNLOAD_DIR "${LIBEVENT_PREFIX_DIR}/download"
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
             -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
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

set(MEMCACHED_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/memcached-src)
FetchContent_Populate(
  memcached
  URL https://github.com/memcached/memcached/archive/refs/tags/1.4.4.tar.gz
  URL_HASH
    SHA256=f695c7127364481e63636653f5f07b3b1a75d4c956c99a20e19053353dee3164
  PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/embed-client.patch
                DOWNLOAD_EXTRACT_TIMESTAMP YES)

set(memcached_sources
    client.c
    ${MEMCACHED_SOURCE_DIR}/assoc.c
    ${MEMCACHED_SOURCE_DIR}/cache.c
    ${MEMCACHED_SOURCE_DIR}/daemon.c
    ${MEMCACHED_SOURCE_DIR}/hash.c
    ${MEMCACHED_SOURCE_DIR}/items.c
    ${MEMCACHED_SOURCE_DIR}/memcached.c
    ${MEMCACHED_SOURCE_DIR}/slabs.c
    ${MEMCACHED_SOURCE_DIR}/stats.c
    ${MEMCACHED_SOURCE_DIR}/thread.c
    ${MEMCACHED_SOURCE_DIR}/util.c)

add_executable(memcached-127 ${memcached_sources})
add_dependencies(memcached-127 libevent)
target_include_directories(
  memcached-127
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${MEMCACHED_SOURCE_DIR} #
          ${LIBEVENT_SOURCE_DIR}/include #
          ${LIBEVENT_GENERATED_INCLUDE_DIR})
target_link_directories(memcached-127 PRIVATE ${LIBEVENT_BINARY_DIR}/lib)
target_link_libraries(memcached-127 event pthread m)
target_compile_definitions(
  memcached-127 PRIVATE -DPACKAGE=\"memcached\" -DVERSION=\"1.4.4\"
                        -D_GNU_SOURCE -DIOV_MAX=1024 -DHAVE_SIGIGNORE)
target_compile_options(memcached-127 PRIVATE -Wno-deprecated-declarations)

test_big_endian(IS_BIG_ENDIAN)
if(NOT IS_BIG_ENDIAN)
  target_compile_definitions(memcached-127 PRIVATE -DENDIAN_LITTLE=1)
endif()

if(APPLE)
  target_compile_definitions(memcached-127 PRIVATE HAVE_HTONLL=1)
endif()
