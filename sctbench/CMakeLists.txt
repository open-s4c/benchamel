if(${BENCHAMEL_BUILD_TSAN})
  add_compile_options(-fsanitize=thread -g -fno-omit-frame-pointer)
  link_libraries(-fsanitize=thread)
endif()

# chess

file(GLOB SRCS chess/*.cpp)
foreach(SRC ${SRCS})
  get_filename_component(NAME ${SRC} NAME_WLE)
  set(TARGET chess.${NAME})
  add_executable(${TARGET} ${SRC})
  target_link_libraries(${TARGET} pthread)
endforeach()

# conc-bugs/aget

# for now we disable some sctbench tests on macOS
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  file(GLOB SRCS conc-bugs/aget-bug2/*.c)
  set(TARGET CB.aget)
  add_executable(${TARGET} ${SRCS})
  target_link_libraries(${TARGET} pthread)
  configure_file(conc-bugs/aget-bug2/0 0 COPYONLY)
  configure_file(conc-bugs/aget-bug2/17573 17573 COPYONLY)
endif()

# conc-bugs/pbzip2

set(SRCS
    blocksort.c
    huffman.c
    crctable.c
    randtable.c
    compress.c
    decompress.c
    bzlib.c)
list(TRANSFORM SRCS PREPEND conc-bugs/pbzip2-0.9.4/bzip2-1.0.6/)
add_library(bz2 STATIC ${SRCS})
add_executable(CB.pbzip2 conc-bugs/pbzip2-0.9.4/pbzip2-0.9.4/pbzip2.cpp)
target_include_directories(CB.pbzip2 PRIVATE conc-bugs/pbzip2-0.9.4/bzip2-1.0.6)
target_link_libraries(CB.pbzip2 pthread bz2)
configure_file(conc-bugs/pbzip2-0.9.4/test.tar test.tar COPYONLY)

# conc-bugs/stringbuffer

file(GLOB SRCS conc-bugs/stringbuffer-jdk1.4/*.cpp)
add_executable(CB.stringbuffer ${SRCS})
target_link_libraries(CB.stringbuffer pthread)

# concurrent-software-benchmarks
file(GLOB SRCS concurrent-software-benchmarks/*.c)
foreach(SRC ${SRCS})
  get_filename_component(NAME ${SRC} NAME_WLE)
  set(TARGET CS.${NAME})
  add_executable(${TARGET} ${SRC})
  target_link_libraries(${TARGET} pthread)
endforeach()

# inspect_benchmarks
file(GLOB SRCS inspect_benchmarks/*.c)
foreach(SRC ${SRCS})
  get_filename_component(NAME ${SRC} NAME_WLE)
  set(TARGET inspect.${NAME})
  add_executable(${TARGET} ${SRC})
  target_link_libraries(${TARGET} pthread)
endforeach()
configure_file(inspect_benchmarks/bzip_input bzip_input COPYONLY)

# inspect_examples
file(GLOB SRCS inspect_examples/*.c)
foreach(SRC ${SRCS})
  get_filename_component(NAME ${SRC} NAME_WLE)
  set(TARGET inspect.${NAME})
  add_executable(${TARGET} ${SRC})
  target_link_libraries(${TARGET} pthread m)
endforeach()
file(COPY inspect_examples/pfscandir DESTINATION pfscandir)
