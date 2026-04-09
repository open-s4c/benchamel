require_distfile(
  TARGET pbzip2-0.9.4
  URL "http://www.compression.ca/pbzip2/pbzip2-0.9.4.tar.gz"
  HASH "d11a8821a510c26f1fd3254742354729125a5b7d0ce98bb6c195045d19fb966d"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)

require_distfile(
  TARGET bzip2-1.0.6
  URL "https://mirrors.xmission.com/slackware/slackware-12.0/patches/source/bzip2/bzip2-1.0.6.tar.gz"
  HASH "a2848f34fcd5d6cf47def00461fcb528a0484d8edef8208d6d2e2909dc61d9cd"
  SOURCE_DIR_VAR BZIP2_SOURCE_DIR)

copy_asset("test.tar")

add_library(
  pbzip2-0.9.4-bzip2 STATIC
  "${BZIP2_SOURCE_DIR}/blocksort.c"
  "${BZIP2_SOURCE_DIR}/bzlib.c"
  "${BZIP2_SOURCE_DIR}/compress.c"
  "${BZIP2_SOURCE_DIR}/crctable.c"
  "${BZIP2_SOURCE_DIR}/decompress.c"
  "${BZIP2_SOURCE_DIR}/huffman.c"
  "${BZIP2_SOURCE_DIR}/randtable.c")
target_include_directories(pbzip2-0.9.4-bzip2 PUBLIC "${BZIP2_SOURCE_DIR}")

add_executable(pbzip2-0.9.4 "${SOURCE_DIR}/pbzip2.cpp")
target_link_libraries(pbzip2-0.9.4 PRIVATE pbzip2-0.9.4-bzip2 Threads::Threads)
target_compile_definitions(
  pbzip2-0.9.4
  PRIVATE _LARGEFILE64_SOURCE _FILE_OFFSET_BITS=64
          BENCHAMEL_PBZIP2_TEST_TAR="${CMAKE_CURRENT_BINARY_DIR}/test.tar")
target_compile_options(pbzip2-0.9.4 PRIVATE -O3)

configure_bug_target(pbzip2-0.9.4)
