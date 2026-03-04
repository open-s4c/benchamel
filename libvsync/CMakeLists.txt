if (${BENCHAMEL_BUILD_TSAN})
    add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer -DVSYNC_SC)
    link_libraries(-fsanitize=thread)
endif()

file(GLOB SRCS test/queue/*.c test/spinlock/*.c)

foreach(SRC ${SRCS})
	get_filename_component(NAME ${SRC} NAME_WLE)
	set(TARGET ${NAME})
	add_executable(${TARGET} ${SRC})
	target_link_libraries(${TARGET} pthread)
	target_include_directories(${TARGET} PRIVATE include)
endforeach()
