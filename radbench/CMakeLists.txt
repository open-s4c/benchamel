if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

include(ExternalProject)
include(FetchContent)

find_package(Threads REQUIRED)
find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)

set(RADBENCH_C_FLAGS "${CMAKE_C_FLAGS}")
set(RADBENCH_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(RADBENCH_LDFLAGS "${CMAKE_EXE_LINKER_FLAGS}")

if(CMAKE_BUILD_TYPE)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" RADBENCH_BUILD_TYPE_UPPER)
  string(APPEND RADBENCH_C_FLAGS " ${CMAKE_C_FLAGS_${RADBENCH_BUILD_TYPE_UPPER}}")
  string(APPEND RADBENCH_CXX_FLAGS " ${CMAKE_CXX_FLAGS_${RADBENCH_BUILD_TYPE_UPPER}}")
  string(APPEND RADBENCH_LDFLAGS
         " ${CMAKE_EXE_LINKER_FLAGS_${RADBENCH_BUILD_TYPE_UPPER}}")
endif()

if(BENCHAMEL_BUILD_TSAN)
  string(APPEND RADBENCH_C_FLAGS " -fsanitize=thread -g -fno-omit-frame-pointer")
  string(APPEND RADBENCH_CXX_FLAGS
         " -fsanitize=thread -g -fno-omit-frame-pointer")
  string(APPEND RADBENCH_LDFLAGS " -fsanitize=thread")
endif()

set(RADBENCH_NSPR_COMPILE_OPTIONS
    -fvisibility=hidden
    -pipe
    -Wall
    -pthread
    -g
    -fno-inline
    -fPIC
    -UNDEBUG
    -UHAVE_CVAR_BUILT_ON_SEM)
set(RADBENCH_NSPR_COMPILE_DEFINITIONS
    DEBUG_najalber
    DEBUG=1
    HAVE_VISIBILITY_HIDDEN_ATTRIBUTE=1
    HAVE_VISIBILITY_PRAGMA=1
    XP_UNIX=1
    _GNU_SOURCE=1
    HAVE_FCNTL_FILE_LOCKING=1
    LINUX=1
    i386=1
    HAVE_LCHOWN=1
    HAVE_STRERROR=1
    _REENTRANT=1
    FORCE_PR_LOG
    _PR_PTHREADS)

function(set_radbench_properties target output_name)
  set_target_properties(
    "${target}"
    PROPERTIES OUTPUT_NAME "${output_name}" PREFIX "rad.")
endfunction()

function(radbench_fetch_content name url source_dir)
  FetchContent_Declare(
    "${name}"
    URL "${url}"
    DOWNLOAD_EXTRACT_TIMESTAMP YES
    SOURCE_DIR "${source_dir}")

  FetchContent_GetProperties("${name}")
  if(NOT ${name}_POPULATED)
    FetchContent_Populate("${name}")
  endif()
endfunction()

function(radbench_prepare_nspr_tree case_name nspr_source_dir)
  foreach(asm_name IN ITEMS os_Linux_x86.s os_Linux_x86_64.s)
    set(asm_source "${CMAKE_CURRENT_SOURCE_DIR}/${case_name}/src/${asm_name}")
    if(EXISTS "${asm_source}")
      foreach(
        asm_dest_dir
        IN ITEMS "${nspr_source_dir}/mozilla/nsprpub/pr/src/md"
                 "${nspr_source_dir}/mozilla/nsprpub/pr/src/md/unix")
        if(EXISTS "${asm_dest_dir}")
          file(COPY_FILE "${asm_source}" "${asm_dest_dir}/${asm_name}"
               ONLY_IF_DIFFERENT)
        endif()
      endforeach()
    endif()
  endforeach()
endfunction()

function(configure_radbench_nspr_harness target)
  target_compile_options("${target}" PRIVATE ${RADBENCH_NSPR_COMPILE_OPTIONS})
  target_compile_definitions("${target}" PRIVATE ${RADBENCH_NSPR_COMPILE_DEFINITIONS})
endfunction()

function(add_radbench_nspr_case name rel_source nspr_url nspr_dir output_name)
  set(case_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}/src")
  set(nspr_source_dir "${case_src_dir}/${nspr_dir}")
  set(nspr_target_dir "${nspr_source_dir}/target")
  set(nspr_dist_dir "${nspr_target_dir}/dist")
  set(nspr_libnspr4 "${nspr_dist_dir}/lib/libnspr4.a")
  set(nspr_libplc4 "${nspr_dist_dir}/lib/libplc4.a")
  set(nspr_libplds4 "${nspr_dist_dir}/lib/libplds4.a")
  set(fetch_name "radbench_${name}_nspr")
  set(build_target "radbench-${name}-nspr-build")
  set(exec_target "rad-${name}")

  # FETCHCONTENT_SOURCE_DIR_<name> can override this download to reuse a local
  # checkout or cached extraction.
  radbench_fetch_content("${fetch_name}" "${nspr_url}" "${nspr_source_dir}")
  radbench_prepare_nspr_tree("${name}" "${nspr_source_dir}")

  ExternalProject_Add(
    "${build_target}"
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    SOURCE_DIR "${nspr_source_dir}"
    BINARY_DIR "${nspr_target_dir}"
    CONFIGURE_COMMAND
      "${CMAKE_COMMAND}" -E env
      "CC=${CMAKE_C_COMPILER}"
      "CXX=${CMAKE_CXX_COMPILER}"
      "CFLAGS=${RADBENCH_C_FLAGS}"
      "CXXFLAGS=${RADBENCH_CXX_FLAGS}"
      "LDFLAGS=${RADBENCH_LDFLAGS}"
      ../mozilla/nsprpub/configure --enable-64bit --prefix=${nspr_dist_dir}
    BUILD_COMMAND "${MAKE_EXECUTABLE}"
    INSTALL_COMMAND "${MAKE_EXECUTABLE}" install
    BUILD_BYPRODUCTS "${nspr_libnspr4}" "${nspr_libplc4}" "${nspr_libplds4}")

  add_executable("${exec_target}" "${case_src_dir}/${rel_source}")
  add_dependencies("${exec_target}" "${build_target}")
  configure_radbench_nspr_harness("${exec_target}")
  target_include_directories(
    "${exec_target}"
    PRIVATE
      "${nspr_dist_dir}/include/nspr"
      "${nspr_source_dir}/mozilla/nsprpub/pr/include"
      "${nspr_source_dir}/mozilla/nsprpub/pr/include/private")
  target_link_libraries(
    "${exec_target}"
    PRIVATE
      Threads::Threads
      ${CMAKE_DL_LIBS}
      "${nspr_libnspr4}"
      "${nspr_libplc4}"
      "${nspr_libplds4}")
  set_radbench_properties("${exec_target}" "${output_name}")
endfunction()

function(add_radbench_js_case name rel_source nspr_url nspr_dir js_url js_dir
         output_name)
  set(case_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}/src")
  set(nspr_source_dir "${case_src_dir}/${nspr_dir}")
  set(nspr_target_dir "${nspr_source_dir}/target")
  set(nspr_dist_dir "${nspr_target_dir}/dist")
  set(js_source_dir "${case_src_dir}/${js_dir}")
  set(js_build_dir "${js_source_dir}/src")
  set(js_object_dir "${js_build_dir}/Linux_All_DBG.OBJ")
  set(js_library "${js_object_dir}/libjs.so")
  set(nspr_fetch_name "radbench_${name}_nspr")
  set(js_fetch_name "radbench_${name}_js")
  set(nspr_build_target "radbench-${name}-nspr-build")
  set(js_build_target "radbench-${name}-js-build")
  set(exec_target "rad-${name}")
  set(nspr_libnspr4 "${nspr_dist_dir}/lib/libnspr4.a")
  set(nspr_libplc4 "${nspr_dist_dir}/lib/libplc4.a")
  set(nspr_libplds4 "${nspr_dist_dir}/lib/libplds4.a")
  set(js_xcflags "${RADBENCH_CXX_FLAGS} -I${nspr_dist_dir}/include/nspr")
  set(
    js_xldflags
    "${RADBENCH_LDFLAGS} -L${nspr_dist_dir}/lib -lplc4 -L${nspr_dist_dir}/lib -lnspr4 -lpthread"
  )

  radbench_fetch_content("${nspr_fetch_name}" "${nspr_url}" "${nspr_source_dir}")
  radbench_fetch_content("${js_fetch_name}" "${js_url}" "${js_source_dir}")
  radbench_prepare_nspr_tree("${name}" "${nspr_source_dir}")

  ExternalProject_Add(
    "${nspr_build_target}"
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    SOURCE_DIR "${nspr_source_dir}"
    BINARY_DIR "${nspr_target_dir}"
    CONFIGURE_COMMAND
      "${CMAKE_COMMAND}" -E env
      "CC=${CMAKE_C_COMPILER}"
      "CXX=${CMAKE_CXX_COMPILER}"
      "CFLAGS=${RADBENCH_C_FLAGS}"
      "CXXFLAGS=${RADBENCH_CXX_FLAGS}"
      "LDFLAGS=${RADBENCH_LDFLAGS}"
      ../mozilla/nsprpub/configure --enable-64bit --prefix=${nspr_dist_dir}
    BUILD_COMMAND "${MAKE_EXECUTABLE}"
    INSTALL_COMMAND "${MAKE_EXECUTABLE}" install
    BUILD_BYPRODUCTS "${nspr_libnspr4}" "${nspr_libplc4}" "${nspr_libplds4}")

  ExternalProject_Add(
    "${js_build_target}"
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    SOURCE_DIR "${js_source_dir}"
    BINARY_DIR "${js_build_dir}"
    BUILD_COMMAND
      "${CMAKE_COMMAND}" -E env
      "CC=${CMAKE_C_COMPILER}"
      "CXX=${CMAKE_CXX_COMPILER}"
      "CCC=${CMAKE_CXX_COMPILER}"
      "LD_LIBRARY_PATH=${nspr_dist_dir}/lib:$ENV{LD_LIBRARY_PATH}"
      "XCFLAGS=${js_xcflags}"
      "XLDFLAGS=${js_xldflags}"
      "JS_THREADSAFE=1"
      "${MAKE_EXECUTABLE}" -f Makefile.ref
    INSTALL_COMMAND ""
    DEPENDS "${nspr_build_target}"
    BUILD_BYPRODUCTS "${js_library}")

  add_executable("${exec_target}" "${case_src_dir}/${rel_source}")
  add_dependencies("${exec_target}" "${js_build_target}")
  target_compile_definitions("${exec_target}" PRIVATE XP_UNIX JS_THREADSAFE)
  target_include_directories(
    "${exec_target}"
    PRIVATE "${js_source_dir}/src" "${nspr_dist_dir}/include/nspr")
  target_link_libraries(
    "${exec_target}"
    PRIVATE
      Threads::Threads
      ${CMAKE_DL_LIBS}
      "${js_library}"
      "${nspr_libnspr4}"
      "${nspr_libplc4}"
      "${nspr_libplds4}")
  set_target_properties(
    "${exec_target}"
    PROPERTIES BUILD_RPATH "${js_object_dir};${nspr_dist_dir}/lib")
  set_radbench_properties("${exec_target}" "${output_name}")
endfunction()

add_radbench_js_case(
  "bug2"
  "test-ctxt.cpp"
  "https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.7/src/nspr-4.6.7.tar.gz"
  "nspr-4.6.7"
  "https://ftp.mozilla.org/pub/mozilla.org/js/js-1.7.0.tar.gz"
  "js"
  "bug2")
add_radbench_js_case(
  "bug3"
  "test.c"
  "https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.7.3/src/nspr-4.7.3.tar.gz"
  "nspr-4.7.3"
  "https://ftp.mozilla.org/pub/mozilla.org/js/js-1.8.0-rc1.tar.gz"
  "js"
  "bug3")
add_radbench_nspr_case(
  "bug4"
  "threads.c"
  "https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.4/src/nspr-4.6.4.tar.gz"
  "nspr-4.6.4"
  "bug4")
add_radbench_nspr_case(
  "bug5"
  "cvar.c"
  "https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.8.6/src/nspr-4.8.6.tar.gz"
  "nspr-4.8.6"
  "bug5")
add_radbench_nspr_case(
  "bug6"
  "rwlock-test.c"
  "https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.8/src/nspr-4.8.tar.gz"
  "nspr-4.8"
  "bug6")
