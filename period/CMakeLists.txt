find_package(BZip2 REQUIRED)
find_package(Threads REQUIRED)

function(set_period_properties target output_name)
  set_target_properties(
    "${target}"
    PROPERTIES OUTPUT_NAME "${output_name}" PREFIX "period.")
endfunction()

function(copy_period_runtime target rel_source)
  get_filename_component(runtime_name "${rel_source}" NAME)
  add_custom_command(
    TARGET "${target}"
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/${rel_source}"
      "$<TARGET_FILE_DIR:${target}>/${runtime_name}")
endfunction()

add_executable(period-bzip2smp "${CMAKE_CURRENT_SOURCE_DIR}/bzip2smp/bzip2smp.comb.c")
target_link_libraries(period-bzip2smp PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
set_period_properties(period-bzip2smp "bzip2smp")
copy_period_runtime(period-bzip2smp "bzip2smp/bzip_input")

add_library(period-ctrace-lib STATIC "${CMAKE_CURRENT_SOURCE_DIR}/CTrace/src/ctrace.c")
target_include_directories(
  period-ctrace-lib
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/CTrace/inc")
target_link_libraries(period-ctrace-lib PUBLIC Threads::Threads)

add_executable(period-ctrace-test "${CMAKE_CURRENT_SOURCE_DIR}/CTrace/test/main.c")
target_link_libraries(period-ctrace-test PRIVATE period-ctrace-lib Threads::Threads)
set_period_properties(period-ctrace-test "ctrace-test")

add_executable(period-pbzip2-1.0.5 "${CMAKE_CURRENT_SOURCE_DIR}/pbzip2-1.0.5/pbzip2-1.0.5-src/pbzip2.cpp")
target_compile_definitions(
  period-pbzip2-1.0.5
  PRIVATE _LARGEFILE64_SOURCE _FILE_OFFSET_BITS=64)
target_compile_options(
  period-pbzip2-1.0.5
  PRIVATE -O2 -Wno-narrowing -Wno-reserved-user-defined-literal)
target_link_libraries(period-pbzip2-1.0.5 PRIVATE BZip2::BZip2 Threads::Threads)
set_period_properties(period-pbzip2-1.0.5 "pbzip2-1.0.5")
copy_period_runtime(period-pbzip2-1.0.5 "pbzip2-1.0.5/test.tar")

add_executable(
  period-pbzip2-1.1.13
  "${CMAKE_CURRENT_SOURCE_DIR}/pbzip2-1.1.13/pbzip2-1.1.13-src/pbzip2.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pbzip2-1.1.13/pbzip2-1.1.13-src/BZ2StreamScanner.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pbzip2-1.1.13/pbzip2-1.1.13-src/ErrorContext.cpp")
target_compile_definitions(
  period-pbzip2-1.1.13
  PRIVATE
    _LARGEFILE64_SOURCE
    _FILE_OFFSET_BITS=64
    _POSIX_PTHREAD_SEMANTICS
    USE_STACKSIZE_CUSTOMIZATION)
target_compile_options(period-pbzip2-1.1.13 PRIVATE -O2 -Wall)
target_link_libraries(period-pbzip2-1.1.13 PRIVATE BZip2::BZip2 Threads::Threads)
set_period_properties(period-pbzip2-1.1.13 "pbzip2-1.1.13")
copy_period_runtime(period-pbzip2-1.1.13 "pbzip2-1.1.13/test.tar")

foreach(
  missing_case
  IN ITEMS
    axel-2.17.10
    lbzip2
    lrzip
    pixz-1.0.5
    pixz-1.0.7
    xz)
  message(
    STATUS
      "Skipping period/${missing_case}: the upstream source bundle is not fully vendored for a native CMake build.")
endforeach()
