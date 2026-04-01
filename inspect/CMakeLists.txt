find_package(Threads REQUIRED)

function(add_inspect_benchmark name rel_source)
  set(target "inspect-${name}")

  add_executable("${target}" "${CMAKE_CURRENT_SOURCE_DIR}/${rel_source}")
  target_link_libraries("${target}" PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

  set_target_properties(
    "${target}"
    PROPERTIES OUTPUT_NAME "${name}" PREFIX "inspect.")
endfunction()

add_inspect_benchmark("bbuf" "bbuf/bbuf.c")
add_inspect_benchmark("boundedBuffer" "boundedBuffer/boundedBuffer.c")
add_inspect_benchmark("ctrace-test" "ctrace-test/ctrace-test.c")
add_inspect_benchmark("qsort_mt" "qsort_mt/qsort_mt.c")
