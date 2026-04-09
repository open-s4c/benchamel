# chess
file(GLOB SRCS chess/*.cpp)
foreach(SRC ${SRCS})
  get_filename_component(NAME ${SRC} NAME_WLE)
  set(TARGET chess.${NAME})
  add_executable(${TARGET} ${SRC})
  target_link_libraries(${TARGET} pthread)
endforeach()

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
