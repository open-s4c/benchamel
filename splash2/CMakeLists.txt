find_package(Threads REQUIRED)
find_program(M4_EXECUTABLE NAMES m4 REQUIRED)

function(set_splash2_properties target output_name)
  set_target_properties(
    "${target}"
    PROPERTIES OUTPUT_NAME "${output_name}" PREFIX "splash2.")
endfunction()

function(splash2_generate output input macros)
  set(extra_args "${ARGN}")
  string(JOIN " " m4_args ${extra_args})

  if(m4_args)
    set(shell_command "\"${M4_EXECUTABLE}\" ${m4_args} \"${macros}\" \"${input}\" > \"${output}\"")
  else()
    set(shell_command "\"${M4_EXECUTABLE}\" \"${macros}\" \"${input}\" > \"${output}\"")
  endif()

  add_custom_command(
    OUTPUT "${output}"
    COMMAND /bin/sh -c "${shell_command}"
    DEPENDS "${input}" "${macros}"
    VERBATIM)
  set_source_files_properties("${output}" PROPERTIES GENERATED TRUE)
endfunction()

function(add_splash2_single_source name rel_source)
  set(case_dir "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  set(build_dir "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  set(macros "${case_dir}/null_macros/c.m4.null")
  set(generated_source "${build_dir}/${name}.c")
  set(generator_target "splash2-${name}-generated")
  set(target "splash2-${name}")

  file(MAKE_DIRECTORY "${build_dir}")
  splash2_generate("${generated_source}" "${case_dir}/${rel_source}" "${macros}" ${ARGN})

  add_custom_target("${generator_target}" DEPENDS "${generated_source}")
  add_executable("${target}" "${generated_source}")
  add_dependencies("${target}" "${generator_target}")
  target_include_directories("${target}" PRIVATE "${build_dir}")
  target_compile_options("${target}" PRIVATE -O0 -w -Wno-return-type)
  target_link_libraries("${target}" PRIVATE Threads::Threads m)
  set_splash2_properties("${target}" "${name}")
endfunction()

set(barnes_case_dir "${CMAKE_CURRENT_SOURCE_DIR}/barnes")
set(barnes_build_dir "${CMAKE_CURRENT_BINARY_DIR}/barnes")
set(barnes_macros "${barnes_case_dir}/null_macros/c.m4.null")
file(MAKE_DIRECTORY "${barnes_build_dir}")

set(barnes_generated_files)
foreach(header IN ITEMS stdinc.H vectmath.H defs.H code.H)
  get_filename_component(header_base "${header}" NAME_WE)
  set(generated_header "${barnes_build_dir}/${header_base}.h")
  splash2_generate(
    "${generated_header}"
    "${barnes_case_dir}/${header}"
    "${barnes_macros}"
    -s
    -Ulen
    -Uindex)
  list(APPEND barnes_generated_files "${generated_header}")
endforeach()

set(barnes_generated_sources)
foreach(source IN ITEMS code.C code_io.C load.C grav.C getparam.C util.C)
  get_filename_component(source_base "${source}" NAME_WE)
  set(generated_source "${barnes_build_dir}/${source_base}.c")
  splash2_generate(
    "${generated_source}"
    "${barnes_case_dir}/${source}"
    "${barnes_macros}"
    -s
    -Ulen
    -Uindex)
  list(APPEND barnes_generated_files "${generated_source}")
  list(APPEND barnes_generated_sources "${generated_source}")
endforeach()

add_custom_target(splash2-barnes-generated DEPENDS ${barnes_generated_files})
add_executable(splash2-barnes ${barnes_generated_sources})
add_dependencies(splash2-barnes splash2-barnes-generated)
target_include_directories(splash2-barnes PRIVATE "${barnes_build_dir}")
target_compile_options(splash2-barnes PRIVATE -O0 -Wno-return-type)
target_link_libraries(splash2-barnes PRIVATE Threads::Threads m)
set_splash2_properties(splash2-barnes "barnes")
add_custom_command(
  TARGET splash2-barnes
  POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different
    "${barnes_case_dir}/input"
    "$<TARGET_FILE_DIR:splash2-barnes>/input")

add_splash2_single_source(fft "fft.C")
add_splash2_single_source(lu "lu.C")
