require_distfile(
  TARGET transmission-1.42
  URL "https://download.transmissionbt.com/files/transmission-1.42.tar.bz2"
  HASH "e7e17f29a166794017c6d94362159337f14008c414d1f8da5f7c2c1c05c7d543"
  FILENAME "transmission-1.42.tar.bz2"
  PATCHES_DIR patches
  SOURCE_DIR_VAR SOURCE_DIR)
require_distfile(
  TARGET transmission-1.42-curl
  NAME curl-8.19.0
  URL "https://curl.se/download/curl-8.19.0.tar.xz"
  HASH "4eb41489790d19e190d7ac7e18e82857cdd68af8f4e66b292ced562d333f11df"
  FILENAME "curl-8.19.0.tar.xz"
  SOURCE_DIR_VAR CURL_SOURCE_DIR)

# Local paths for the benchmark's input, generated files, and exported binary.
copy_asset("transmission-1.42-input.txt")
set(INPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/transmission-1.42-input.txt")
set(OUTPUT_TORRENT "${CMAKE_CURRENT_BINARY_DIR}/transmission-1.42-output.torrent")
set(CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/transmission-1.42-config")
set(DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/transmission-1.42-download")
set(BINARY "${SOURCE_DIR}/cli/transmissioncli")

# Build a local static curl dependency from the prepared distfile source.
set(CURL_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/curl-build")
set(CURL_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/curl-install")
set(CURL_LIBRARY "${CURL_INSTALL_DIR}/lib/libcurl.a")
set(CURL_INCLUDE_DIR "${CURL_INSTALL_DIR}/include")
set(CURL_LIBS
    "-L${CURL_INSTALL_DIR}/lib -lcurl -lssl -lcrypto -lz -ldl -pthread")

ExternalProject_Add(
  transmission-1.42-curl-build
  SOURCE_DIR "${CURL_SOURCE_DIR}"
  BINARY_DIR "${CURL_BUILD_DIR}"
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  CONFIGURE_COMMAND
    "${CMAKE_COMMAND}" -E env "CFLAGS=${BENCHAMEL_C_FLAGS}"
    "CXXFLAGS=${BENCHAMEL_CXX_FLAGS}" "LDFLAGS=${BENCHAMEL_LDFLAGS}"
    "${CURL_SOURCE_DIR}/configure"
    "--prefix=${CURL_INSTALL_DIR}" "--disable-shared" "--enable-static"
    "--disable-ldap" "--disable-ldaps" "--without-brotli" "--without-zstd"
    "--without-libpsl" "--without-libidn2" "--without-nghttp2"
    "--without-ca-bundle" "--without-ca-path" "--with-openssl"
  BUILD_COMMAND "${CMAKE_MAKE_PROGRAM}" -j1
  INSTALL_COMMAND "${CMAKE_MAKE_PROGRAM}" install
  BUILD_BYPRODUCTS "${CURL_LIBRARY}")

# Build the real transmission CLI binary from the prepared source tree. The
# patch series adds the argc==1 self-exec path with canonical benchmark inputs.
set(C_FLAGS "${BENCHAMEL_C_FLAGS}")
set(
  CPPFLAGS
  "-DBENCHAMEL_TRANSMISSION_INPUT='\"${INPUT_FILE}\"' -DBENCHAMEL_TRANSMISSION_OUTPUT='\"${OUTPUT_TORRENT}\"' -DBENCHAMEL_TRANSMISSION_CONFIG_DIR='\"${CONFIG_DIR}\"' -DBENCHAMEL_TRANSMISSION_DOWNLOAD_DIR='\"${DOWNLOAD_DIR}\"'")

ExternalProject_Add(
  transmission-1.42-build
  SOURCE_DIR "${SOURCE_DIR}"
  BUILD_IN_SOURCE 1
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  CONFIGURE_COMMAND
    "${CMAKE_COMMAND}" -E env "CFLAGS=${C_FLAGS}"
    "CPPFLAGS=${CPPFLAGS}"
    "CXXFLAGS=${BENCHAMEL_CXX_FLAGS}" "LDFLAGS=${BENCHAMEL_LDFLAGS}"
    "${SOURCE_DIR}/configure"
    "LIBCURL_CFLAGS=-I${CURL_INCLUDE_DIR}"
    "LIBCURL_LIBS=${CURL_LIBS}" "--disable-gtk" "--disable-wx"
    "--disable-beos" "--disable-mac" "--disable-daemon" "--disable-nls"
    "--enable-cli"
  BUILD_COMMAND "${CMAKE_MAKE_PROGRAM}" -j1 "CPPFLAGS=${CPPFLAGS}"
  BUILD_BYPRODUCTS "${BINARY}"
  DEPENDS transmission-1.42-curl-build)

add_custom_target(
  transmission-1.42-runtime
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CONFIG_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOWNLOAD_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E rm -f "${OUTPUT_TORRENT}"
  DEPENDS transmission-1.42-build "${INPUT_FILE}"
  VERBATIM)

add_custom_target(transmission-1.42 ALL DEPENDS transmission-1.42-runtime)
register_bug_target(transmission-1.42 "${BINARY}")

# Ensure export never races ahead of ExternalProject output generation on
# first parallel builds.
add_dependencies(transmission-1.42-export transmission-1.42-build)
