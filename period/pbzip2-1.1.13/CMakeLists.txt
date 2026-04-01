find_package(BZip2 REQUIRED)
find_package(Threads REQUIRED)

set(TEST_TAR "${CMAKE_CURRENT_BINARY_DIR}/test.tar")

copy_asset("test.tar")

add_executable(
  pbzip2-1.1.13
  "${CMAKE_CURRENT_SOURCE_DIR}/pbzip2-1.1.13-src/pbzip2.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pbzip2-1.1.13-src/BZ2StreamScanner.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pbzip2-1.1.13-src/ErrorContext.cpp")

target_compile_definitions(
  pbzip2-1.1.13
  PRIVATE
    _LARGEFILE64_SOURCE
    _FILE_OFFSET_BITS=64
    _POSIX_PTHREAD_SEMANTICS
    USE_STACKSIZE_CUSTOMIZATION
    BENCHAMEL_PERIOD_PBZIP2_113_TEST_TAR="${TEST_TAR}")

target_compile_options(pbzip2-1.1.13 PRIVATE -O2 -Wall)
target_link_libraries(pbzip2-1.1.13 PRIVATE BZip2::BZip2 Threads::Threads)

set_target_properties(
  pbzip2-1.1.13
  PROPERTIES PREFIX "period." RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
