set(AGET_BUG1_CASE_DIR "${jcbugs_SOURCE_DIR}/aget-bug1")
set(AGET_BUG1_PATCHES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/patches")

# Leave the upstream aget-devel URL open for now. The local distfiles cache or
# BENCHAMEL_DISTFILES_BASE_URL is expected to provide aget-devel.tar.gz.
set(BENCHAMEL_AGET_DEVEL_URL
    "https://example.invalid/aget-devel.tar.gz"
    CACHE STRING "Fallback upstream URL for aget-devel.tar.gz")

require_distfile(
  TARGET aget-bug1-source
  NAME aget-devel
  URL "${BENCHAMEL_AGET_DEVEL_URL}"
  HASH "d572456fcec74c57116feeea8868dcc900f8a3b99d2736ed624c3cc59ff15080"
  OUT_ARCHIVE_VAR AGET_BUG1_ARCHIVE
  SOURCE_DIR_VAR AGET_BUG1_SOURCE_DIR)

set(AGET_BUG1_PREPARED_STAMP "${AGET_BUG1_SOURCE_DIR}/.benchamel-prepared")
file(SHA256 "${AGET_BUG1_PATCHES_DIR}/aget-bug1.patch"
     AGET_BUG1_PATCH_SHA256)
set(AGET_BUG1_PREPARE_SIGNATURE "patchmode=target-helper-v1\ncase=${AGET_BUG1_PATCH_SHA256}\n")

set(AGET_BUG1_NEEDS_PREPARE TRUE)
if(EXISTS "${AGET_BUG1_PREPARED_STAMP}")
  file(READ "${AGET_BUG1_PREPARED_STAMP}" AGET_BUG1_PREPARED_SIGNATURE)
  if(AGET_BUG1_PREPARED_SIGNATURE STREQUAL AGET_BUG1_PREPARE_SIGNATURE)
    set(AGET_BUG1_NEEDS_PREPARE FALSE)
  endif()
endif()

if(AGET_BUG1_NEEDS_PREPARE)
  file(REMOVE_RECURSE "${AGET_BUG1_SOURCE_DIR}")
  require_distfile(
    TARGET aget-bug1-source
    NAME aget-devel
    URL "${BENCHAMEL_AGET_DEVEL_URL}"
    HASH "d572456fcec74c57116feeea8868dcc900f8a3b99d2736ed624c3cc59ff15080"
    OUT_ARCHIVE_VAR AGET_BUG1_ARCHIVE
    SOURCE_DIR_VAR AGET_BUG1_SOURCE_DIR)

  execute_process(
    COMMAND patch -p1 -i "${AGET_BUG1_PATCHES_DIR}/aget-bug1.patch"
    WORKING_DIRECTORY "${AGET_BUG1_SOURCE_DIR}"
    RESULT_VARIABLE aget_bug1_patch_rv)
  if(NOT aget_bug1_patch_rv EQUAL 0)
    message(FATAL_ERROR "failed to apply aget-bug1 patch")
  endif()

  file(WRITE "${AGET_BUG1_PREPARED_STAMP}" "${AGET_BUG1_PREPARE_SIGNATURE}")
endif()

add_custom_target(aget-bug1-source DEPENDS "${AGET_BUG1_PREPARED_STAMP}")
