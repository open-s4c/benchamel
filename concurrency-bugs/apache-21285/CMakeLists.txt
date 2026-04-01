require_distfile(
  TARGET apache-21285
  NAME httpd-2.0.46
  URL "https://archive.apache.org/dist/httpd/httpd-2.0.46.tar.gz"
  HASH "3227bdb190a5ca29d47b55ed146c53cfcc7beb1fee3a051c0c7fe162e491d3b2"
  FILENAME "httpd-2.0.46.tar.gz"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)

set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")
set(CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/httpd.conf")
set(GENERATED_MODULE "${CMAKE_CURRENT_BINARY_DIR}/mod_benchamel_21285.c")
set(BINARY "${INSTALL_DIR}/bin/httpd")
set(APXS "${INSTALL_DIR}/bin/apxs")
set(MODULE "${INSTALL_DIR}/modules/mod_benchamel_21285.so")
set(EXPORTED_BINARY "${CONCURRENCY_BUGS_BIN_DIR}/JCB.apache-21285")

set(
  CPPFLAGS
  "-DBENCHAMEL_APACHE_21285_SERVER_ROOT='\"${INSTALL_DIR}\"' -DBENCHAMEL_APACHE_21285_CONFIG_FILE='\"${INSTALL_DIR}/conf/httpd.conf\"'")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/apache-21285.conf.in"
               "${CONFIG_FILE}" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mod_benchamel_21285.c.in"
               "${GENERATED_MODULE}" @ONLY)

ExternalProject_Add(
  apache-21285-build
  SOURCE_DIR "${SOURCE_DIR}"
  BUILD_IN_SOURCE 1
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  CONFIGURE_COMMAND
    "${CMAKE_COMMAND}" -E env "ac_cv_sizeof_short=2" "ac_cv_sizeof_int=4"
    "ac_cv_sizeof_long=8" "ac_cv_sizeof_off_t=8" "ac_cv_sizeof_size_t=8"
    "ac_cv_sizeof_ssize_t=8" "ac_cv_sizeof_pid_t=4"
    "ac_cv_struct_rlimit=yes" "${SOURCE_DIR}/configure"
    "--prefix=${INSTALL_DIR}" "--with-mpm=worker" "--enable-so"
    "--enable-cache=shared" "--enable-mem-cache=shared" "--disable-cgi"
    "--disable-cgid"
  BUILD_COMMAND "${CMAKE_MAKE_PROGRAM}" -j1 "CFLAGS=${CONCURRENCY_BUGS_C_FLAGS}"
                "CPPFLAGS=${CPPFLAGS}" "CXXFLAGS=${CONCURRENCY_BUGS_CXX_FLAGS}"
                "LDFLAGS=${CONCURRENCY_BUGS_LDFLAGS}"
  INSTALL_COMMAND "${CMAKE_MAKE_PROGRAM}" install
  BUILD_BYPRODUCTS "${BINARY}" "${APXS}"
                  "${INSTALL_DIR}/modules/mod_cache.so"
                  "${INSTALL_DIR}/modules/mod_mem_cache.so")

add_custom_command(
  OUTPUT "${EXPORTED_BINARY}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALL_DIR}/logs"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CONFIG_FILE}"
          "${INSTALL_DIR}/conf/httpd.conf"
  COMMAND "${CMAKE_COMMAND}" -E touch "${INSTALL_DIR}/logs/error_log"
  COMMAND
    "${CMAKE_COMMAND}" -E env "CFLAGS=${CONCURRENCY_BUGS_C_FLAGS}"
    "LDFLAGS=${CONCURRENCY_BUGS_LDFLAGS}" "${APXS}" -c -i -n
    benchamel_21285 "${GENERATED_MODULE}"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${BINARY}"
          "${EXPORTED_BINARY}"
  DEPENDS apache-21285-build "${CONFIG_FILE}" "${GENERATED_MODULE}"
  VERBATIM)

add_custom_target(apache-21285 ALL DEPENDS "${EXPORTED_BINARY}")
register_bug_target(apache-21285 "${EXPORTED_BINARY}")
