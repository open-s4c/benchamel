if(${BENCHAMEL_BUILD_TSAN})
  add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer)
  link_libraries(-fsanitize=thread)
endif()

add_executable(buf_ring01 test.c)
target_link_libraries(buf_ring01 pthread m)

add_executable(buf_ring02 test.c)
target_link_libraries(buf_ring02 pthread m)
target_compile_definitions(buf_ring02 PRIVATE BUGFIX1)
