require_distfile(
  TARGET cherokee-bug1
  NAME cherokee-0.9.2
  URL "http://www.cherokee-project.com/download/0.9/0.9.2/cherokee-0.9.2.tar.gz"
  HASH "b4eab6ec6817e93bcd48e10ade29cb9cf5212febaa8055f66fd11aa8f4fad71e"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)

set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")
set(CHEROKEE_BUG1_INSTALL_DIR "${INSTALL_DIR}")
set(CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/cherokee.conf")
set(INSTALLED_CONFIG "${INSTALL_DIR}/etc/cherokee/cherokee.conf")
set(WORKER "${INSTALL_DIR}/sbin/cherokee-worker")
set(EXPORTED_BINARY "${CONCURRENCY_BUGS_BIN_DIR}/JCB.cherokee-bug1")
set(C_FLAGS "${CONCURRENCY_BUGS_C_FLAGS} -std=gnu11")
set(
  CPPFLAGS
  "-DBENCHAMEL_CHEROKEE_BUG1_ACCESS_LOG='\"${INSTALL_DIR}/var/log/cherokee.access\"'")
set(LDFLAGS "${CONCURRENCY_BUGS_LDFLAGS} -Wl,-rpath,${INSTALL_DIR}/lib")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cherokee-bug1.conf.in"
               "${CONFIG_FILE}" @ONLY)

# Prepare the extracted tree with the embedded client sources compiled into
# cherokee-worker by the local patch series.
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

# Configure, build, and install Cherokee into a local prefix used only by this
# benchmark. The patched cherokee-worker is the real benchmark binary.
add_custom_command(
  OUTPUT "${WORKER}"
  COMMAND
    "${CMAKE_COMMAND}" -E env "CFLAGS=${C_FLAGS}"
    "CXXFLAGS=${CONCURRENCY_BUGS_CXX_FLAGS}" "LDFLAGS=${LDFLAGS}"
    "${SOURCE_DIR}/configure" "--prefix=${INSTALL_DIR}"
  COMMAND "${CMAKE_MAKE_PROGRAM}" constants.h cherokee.conf.sample
          performance.conf.sample
  COMMAND "${CMAKE_MAKE_PROGRAM}" -C cherokee "CPPFLAGS=${CPPFLAGS}"
  COMMAND "${CMAKE_MAKE_PROGRAM}" -C cherokee "CPPFLAGS=${CPPFLAGS}" install
  COMMAND "${CMAKE_MAKE_PROGRAM}" install-data
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALL_DIR}/var/log"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALL_DIR}/var/run"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CONFIG_FILE}"
          "${INSTALLED_CONFIG}"
  COMMAND "${CMAKE_COMMAND}" -E touch "${INSTALL_DIR}/var/log/cherokee.access"
  COMMAND "${CMAKE_COMMAND}" -E touch "${INSTALL_DIR}/var/log/cherokee.error"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  DEPENDS "${SOURCE_DIR}/cherokee/cherokee-client.c"
          "${SOURCE_DIR}/cherokee/cherokee-client.h" "${CONFIG_FILE}"
  VERBATIM)

# Export the real worker binary under the Benchamel naming convention and copy
# the generated config file alongside it for the argc==1 self-exec path.
add_custom_command(
  OUTPUT "${EXPORTED_BINARY}"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${WORKER}"
          "${EXPORTED_BINARY}"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CONFIG_FILE}"
          "${CONCURRENCY_BUGS_BIN_DIR}/cherokee.conf"
  DEPENDS "${WORKER}"
  VERBATIM)

add_custom_target(cherokee-bug1 ALL DEPENDS "${EXPORTED_BINARY}")
register_bug_target(cherokee-bug1 "${EXPORTED_BINARY}")
