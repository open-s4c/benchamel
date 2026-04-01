set(CHEROKEE_092_CASE_DIR "${jcbugs_SOURCE_DIR}/cherokee-0.9.2")
set(CHEROKEE_092_PATCHES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/patches")

require_distfile(
  TARGET cherokee-0.9.2-source
  NAME cherokee-0.9.2
  URL "http://www.cherokee-project.com/download/0.9/0.9.2/cherokee-0.9.2.tar.gz"
  HASH "b4eab6ec6817e93bcd48e10ade29cb9cf5212febaa8055f66fd11aa8f4fad71e"
  OUT_ARCHIVE_VAR CHEROKEE_092_ARCHIVE
  SOURCE_DIR_VAR CHEROKEE_092_SOURCE_DIR)

set(CHEROKEE_092_PREPARED_STAMP
    "${CHEROKEE_092_SOURCE_DIR}/.benchamel-prepared")
file(SHA256 "${CHEROKEE_092_PATCHES_DIR}/cherokee-0.9.2.patch"
     CHEROKEE_092_PATCH_SHA256)
set(CHEROKEE_092_PREPARE_SIGNATURE
    "patchmode=target-helper-v1\ncase=${CHEROKEE_092_PATCH_SHA256}\n")

set(CHEROKEE_092_NEEDS_PREPARE TRUE)
if(EXISTS "${CHEROKEE_092_PREPARED_STAMP}")
  file(READ "${CHEROKEE_092_PREPARED_STAMP}" CHEROKEE_092_PREPARED_SIGNATURE)
  if(CHEROKEE_092_PREPARED_SIGNATURE STREQUAL
     CHEROKEE_092_PREPARE_SIGNATURE)
    set(CHEROKEE_092_NEEDS_PREPARE FALSE)
  endif()
endif()

if(CHEROKEE_092_NEEDS_PREPARE)
  file(REMOVE_RECURSE "${CHEROKEE_092_SOURCE_DIR}")
  require_distfile(
    TARGET cherokee-0.9.2-source
    NAME cherokee-0.9.2
    URL "http://www.cherokee-project.com/download/0.9/0.9.2/cherokee-0.9.2.tar.gz"
    HASH "b4eab6ec6817e93bcd48e10ade29cb9cf5212febaa8055f66fd11aa8f4fad71e"
    OUT_ARCHIVE_VAR CHEROKEE_092_ARCHIVE
    SOURCE_DIR_VAR CHEROKEE_092_SOURCE_DIR)

  execute_process(
    COMMAND patch -p1 -i "${CHEROKEE_092_PATCHES_DIR}/cherokee-0.9.2.patch"
    WORKING_DIRECTORY "${CHEROKEE_092_SOURCE_DIR}"
    RESULT_VARIABLE cherokee_092_patch_rv)
  if(NOT cherokee_092_patch_rv EQUAL 0)
    message(FATAL_ERROR "failed to apply cherokee-0.9.2 patch")
  endif()

  file(WRITE "${CHEROKEE_092_PREPARED_STAMP}"
       "${CHEROKEE_092_PREPARE_SIGNATURE}")
endif()

add_custom_target(cherokee-0.9.2-source DEPENDS "${CHEROKEE_092_PREPARED_STAMP}")
