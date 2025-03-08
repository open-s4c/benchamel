# chess

file(GLOB SRCS chess/*.cpp)
foreach(SRC ${SRCS})
    get_filename_component(NAME ${SRC} NAME_WLE)
	set(TARGET ${NAME})
	add_executable(${TARGET} ${SRC})
	target_link_libraries(${TARGET} pthread)
endforeach()

# aget

# for now we disable some sctbench tests on macOS
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    file(GLOB SRCS conc-bugs/aget-bug2/*.c)
    add_executable(aget ${SRCS})
    target_link_libraries(aget pthread)
    configure_file(conc-bugs/aget-bug2/0 0 COPYONLY)
    configure_file(conc-bugs/aget-bug2/17573 17573 COPYONLY)
endif()

# pbzip2

set(SRCS blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c)
list(TRANSFORM SRCS PREPEND conc-bugs/pbzip2-0.9.4/bzip2-1.0.6/)
add_library(bz2 STATIC ${SRCS})
add_executable(pbzip2 conc-bugs/pbzip2-0.9.4/pbzip2-0.9.4/pbzip2.cpp)
target_include_directories(pbzip2 PRIVATE conc-bugs/pbzip2-0.9.4/bzip2-1.0.6)
target_link_libraries(pbzip2 pthread bz2)
configure_file(conc-bugs/pbzip2-0.9.4/test.tar test.tar COPYONLY)

# conc-bugs/stringbuffer

file(GLOB SRCS conc-bugs/stringbuffer-jdk1.4/*.cpp)
add_executable(stringbuffer ${SRCS})
target_link_libraries(stringbuffer pthread)
