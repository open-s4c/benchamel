require_distfile(
  TARGET httrack-3.43.9
  URL "https://download.httrack.com/httrack-3.43.9.tar.gz"
  HASH "9c10237c1d90f3589a5b9d97a13b992dc6300ad219d1b503c2bfca85594626f0"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)

# Local paths for the installed library and exported benchmark binary.
set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")
set(BINARY "${SOURCE_DIR}/src/.libs/htsserver")
set(LIBRARY "${SOURCE_DIR}/src/.libs/libhttrack.so.2.0.43")
set(INSTALL_LIBRARY_DIR "${INSTALL_DIR}/lib")
set(
  C_FLAGS
  "${BENCHAMEL_C_FLAGS} -DBENCHAMEL_HTTRACK_ROOT='\"${SOURCE_DIR}\"'")
set(
  LDFLAGS
  "${BENCHAMEL_LDFLAGS} -Wl,-rpath,${INSTALL_LIBRARY_DIR}")

# Prepare the extracted tree with the embedded client sources compiled into
# htsserver by the local patch series.
add_custom_command(
  OUTPUT "${SOURCE_DIR}/src/httrack-client.c" "${SOURCE_DIR}/src/httrack-client.h"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/httrack-client.c"
          "${SOURCE_DIR}/src/httrack-client.c"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/httrack-client.h"
          "${SOURCE_DIR}/src/httrack-client.h"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/httrack-client.c"
          "${CMAKE_CURRENT_SOURCE_DIR}/httrack-client.h"
  VERBATIM)

# Configure and build the real htsserver binary. The patch series adds the
# argc==1 self-exec path and in-process client driver.
add_custom_command(
  OUTPUT "${BINARY}"
  COMMAND
    "${CMAKE_COMMAND}" -E env "CFLAGS=${C_FLAGS}"
    "CXXFLAGS=${BENCHAMEL_CXX_FLAGS}"
    "LDFLAGS=${LDFLAGS}" "${SOURCE_DIR}/configure" "--prefix=${INSTALL_DIR}"
  COMMAND "${CMAKE_MAKE_PROGRAM}" -C src libhttrack.la
  COMMAND "${CMAKE_MAKE_PROGRAM}" -C src htsserver
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALL_LIBRARY_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${LIBRARY}"
          "${INSTALL_LIBRARY_DIR}/libhttrack.so.2.0.43"
  COMMAND "${CMAKE_COMMAND}" -E rm -f "${INSTALL_LIBRARY_DIR}/libhttrack.so.2"
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "libhttrack.so.2.0.43"
          "${INSTALL_LIBRARY_DIR}/libhttrack.so.2"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  DEPENDS "${SOURCE_DIR}/src/httrack-client.c"
          "${SOURCE_DIR}/src/httrack-client.h"
  VERBATIM)

add_custom_target(httrack-3.43.9 ALL DEPENDS "${BINARY}")
register_bug_target(httrack-3.43.9 "${BINARY}")
