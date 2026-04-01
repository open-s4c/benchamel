if(${BENCHAMEL_BUILD_TSAN})
  add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer)
  link_libraries(-fsanitize=thread)
endif()

add_executable(freebsd_buf_ring01 test.c)
set_target_properties(freebsd_buf_ring01 PROPERTIES PREFIX "misc.")
target_link_libraries(freebsd_buf_ring01 pthread m)

add_executable(freebsd_buf_ring02 test.c)
set_target_properties(freebsd_buf_ring02 PROPERTIES PREFIX "misc.")
target_link_libraries(freebsd_buf_ring02 pthread m)
target_compile_definitions(freebsd_buf_ring02 PRIVATE BUGFIX1)
