require_distfile(
  TARGET cherokee-bug1
  NAME cherokee-0.9.2
  URL "http://www.cherokee-project.com/download/0.9/0.9.2/cherokee-0.9.2.tar.gz"
  HASH "b4eab6ec6817e93bcd48e10ade29cb9cf5212febaa8055f66fd11aa8f4fad71e"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)

set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CONFIG_FILE "${CMAKE_BINARY_DIR}/bin/cherokee-bug1.conf")
set(WORKER_PATH "${INSTALL_DIR}/sbin/cherokee-worker")
set(C_FLAGS "${BENCHAMEL_C_FLAGS} -std=gnu11")
set(
  CPPFLAGS
  "-DBENCHAMEL_CHEROKEE_BUG1_ACCESS_LOG='\"${INSTALL_DIR}/var/log/cherokee.access\"'")
set(LDFLAGS "${BENCHAMEL_LDFLAGS} -Wl,-rpath,${INSTALL_DIR}/lib")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cherokee-bug1.conf.in"
               "${CONFIG_FILE}" @ONLY)

add_custom_command(
  OUTPUT "${SOURCE_DIR}/cherokee/cherokee-client.c"
         "${SOURCE_DIR}/cherokee/cherokee-client.h"
  COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/cherokee-client.c"
    "${SOURCE_DIR}/cherokee/cherokee-client.c"
  COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/cherokee-client.h"
    "${SOURCE_DIR}/cherokee/cherokee-client.h"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cherokee-client.c"
          "${CMAKE_CURRENT_SOURCE_DIR}/cherokee-client.h"
  VERBATIM)

add_custom_command(
  OUTPUT "${WORKER_PATH}"
  COMMAND
    "${CMAKE_COMMAND}" -E env "CFLAGS=${C_FLAGS}"
    "CXXFLAGS=${BENCHAMEL_CXX_FLAGS}" "LDFLAGS=${LDFLAGS}"
    "${SOURCE_DIR}/configure" "--prefix=${INSTALL_DIR}"
    "--disable-tls"
  COMMAND "${CMAKE_MAKE_PROGRAM}" constants.h cherokee.conf.sample
          performance.conf.sample
  COMMAND "${CMAKE_MAKE_PROGRAM}" -C cherokee "CPPFLAGS=${CPPFLAGS}"
  COMMAND "${CMAKE_MAKE_PROGRAM}" -C cherokee "CPPFLAGS=${CPPFLAGS}" install
  COMMAND "${CMAKE_MAKE_PROGRAM}" install-data
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALL_DIR}/var/log"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALL_DIR}/var/run"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  DEPENDS "${SOURCE_DIR}/cherokee/cherokee-client.c"
          "${SOURCE_DIR}/cherokee/cherokee-client.h" "${CONFIG_FILE}"
  VERBATIM)

add_custom_target(cherokee-bug1 ALL DEPENDS "${WORKER_PATH}" "${CONFIG_FILE}")
register_bug_target(cherokee-bug1 "${WORKER_PATH}")
