require_distfile(
  TARGET apache-25520
  NAME httpd-2.0.48
  URL "https://archive.apache.org/dist/httpd/httpd-2.0.48.tar.gz"
  HASH "b1a08d096ae4c641fe878d1f4129d4e6ee264bc53e3858b8080f8365991e0162"
  FILENAME "httpd-2.0.48.tar.gz"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)

set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")
set(CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/httpd.conf")
set(GENERATED_MODULE "${CMAKE_CURRENT_BINARY_DIR}/mod_benchamel_25520.c")
set(BINARY "${INSTALL_DIR}/bin/httpd")
set(APXS "${INSTALL_DIR}/bin/apxs")

set(
  CPPFLAGS
  "-DBENCHAMEL_APACHE_25520_SERVER_ROOT='\"${INSTALL_DIR}\"' -DBENCHAMEL_APACHE_25520_CONFIG_FILE='\"${INSTALL_DIR}/conf/httpd.conf\"'")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/apache-25520.conf.in"
               "${CONFIG_FILE}" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mod_benchamel_25520.c.in"
               "${GENERATED_MODULE}" @ONLY)

ExternalProject_Add(
  apache-25520-build
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
    "--disable-cgi" "--disable-cgid"
  BUILD_COMMAND "${CMAKE_MAKE_PROGRAM}" -j1 "CFLAGS=${BENCHAMEL_C_FLAGS}"
                "CPPFLAGS=${CPPFLAGS}" "CXXFLAGS=${BENCHAMEL_CXX_FLAGS}"
                "LDFLAGS=${BENCHAMEL_LDFLAGS}"
  INSTALL_COMMAND "${CMAKE_MAKE_PROGRAM}" install
  BUILD_BYPRODUCTS "${BINARY}" "${APXS}")

add_custom_target(
  apache-25520-runtime
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${INSTALL_DIR}/logs"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CONFIG_FILE}"
          "${INSTALL_DIR}/conf/httpd.conf"
  COMMAND "${CMAKE_COMMAND}" -E touch "${INSTALL_DIR}/logs/access_log"
  COMMAND "${CMAKE_COMMAND}" -E touch "${INSTALL_DIR}/logs/error_log"
  COMMAND
    "${CMAKE_COMMAND}" -E env "CFLAGS=${BENCHAMEL_C_FLAGS}"
    "LDFLAGS=${BENCHAMEL_LDFLAGS}" "${APXS}" -c -i -n
    benchamel_25520 "${GENERATED_MODULE}"
  DEPENDS apache-25520-build "${CONFIG_FILE}" "${GENERATED_MODULE}"
  VERBATIM)

add_custom_target(apache-25520 ALL DEPENDS apache-25520-runtime)
register_bug_target(apache-25520 "${BINARY}")
