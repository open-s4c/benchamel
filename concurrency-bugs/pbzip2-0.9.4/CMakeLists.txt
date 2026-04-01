require_distfile(
  TARGET pbzip2-0.9.4
  URL "http://www.compression.ca/pbzip2/pbzip2-0.9.4.tar.gz"
  HASH "d11a8821a510c26f1fd3254742354729125a5b7d0ce98bb6c195045d19fb966d"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)
copy_asset("test.tar")

add_executable(pbzip2-0.9.4 "${SOURCE_DIR}/pbzip2.cpp")
target_link_libraries(pbzip2-0.9.4 PRIVATE BZip2::BZip2 Threads::Threads)
target_compile_definitions(
  pbzip2-0.9.4
  PRIVATE _LARGEFILE64_SOURCE _FILE_OFFSET_BITS=64
          BENCHAMEL_PBZIP2_TEST_TAR="${CMAKE_CURRENT_BINARY_DIR}/test.tar")
target_compile_options(pbzip2-0.9.4 PRIVATE -O3)

configure_bug_target(pbzip2-0.9.4)
