set(BENCHAMEL_AGET_DEVEL_URL
    "https://example.invalid/aget-devel.tar.gz"
    CACHE STRING "Fallback upstream URL for aget-devel.tar.gz")

require_distfile(
  TARGET aget-bug2
  NAME aget-devel
  URL "${BENCHAMEL_AGET_DEVEL_URL}"
  HASH "d572456fcec74c57116feeea8868dcc900f8a3b99d2736ed624c3cc59ff15080"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)

copy_asset("inputs/0")
copy_asset("inputs/17573")

set(aget_bug2_sources
    "${SOURCE_DIR}/main.c"
    "${SOURCE_DIR}/Aget.c"
    "${SOURCE_DIR}/Misc.c"
    "${SOURCE_DIR}/Head.c"
    "${SOURCE_DIR}/Signal.c"
    "${SOURCE_DIR}/Download.c"
    "${SOURCE_DIR}/Ftp.c"
    "${SOURCE_DIR}/Resume.c"
    "${SOURCE_DIR}/loadrc.c")

add_executable(aget-bug2 ${aget_bug2_sources})
target_include_directories(aget-bug2 PRIVATE "${SOURCE_DIR}")
target_compile_definitions(
  aget-bug2
  PRIVATE BENCHAMEL_AGET_BUG2_INPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
          BENCHAMEL_AGET_BUG2_OUTPUT_PATH="${CMAKE_CURRENT_BINARY_DIR}/dl.txt")
target_link_libraries(aget-bug2 PRIVATE Threads::Threads)

configure_bug_target(aget-bug2)
