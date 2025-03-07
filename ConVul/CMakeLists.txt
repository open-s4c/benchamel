# for now we disable all convul tests on macOS
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    file(GLOB SRCS cve-benchmark/*.cpp)
endif()

foreach(SRC ${SRCS})
    get_filename_component(NAME ${SRC} NAME_WLE)
	set(TARGET ${NAME})
	add_executable(${TARGET} ${SRC})
	target_link_libraries(${TARGET} pthread)
endforeach()
