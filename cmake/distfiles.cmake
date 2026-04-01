include_guard(GLOBAL)

function(_benchamel_distfile_try_download url archive_path expected_hash)
  set(tmp_archive_path "${archive_path}.tmp")
  file(REMOVE "${tmp_archive_path}")
  file(
    DOWNLOAD "${url}" "${tmp_archive_path}"
    EXPECTED_HASH "SHA256=${expected_hash}"
    SHOW_PROGRESS
    STATUS download_status
    LOG download_log)
  list(GET download_status 0 download_status_code)
  list(GET download_status 1 download_status_text)
  if(download_status_code EQUAL 0)
    file(RENAME "${tmp_archive_path}" "${archive_path}")
    set(BENCHAMEL_DISTFILE_DOWNLOAD_LOG ""
        PARENT_SCOPE)
    set(BENCHAMEL_DISTFILE_DOWNLOAD_OK TRUE
        PARENT_SCOPE)
    return()
  endif()

  file(REMOVE "${tmp_archive_path}")
  set(
    BENCHAMEL_DISTFILE_DOWNLOAD_LOG
    "failed to download ${url}: ${download_status_text}\n${download_log}"
    PARENT_SCOPE)
  set(BENCHAMEL_DISTFILE_DOWNLOAD_OK FALSE
      PARENT_SCOPE)
endfunction()

# require_distfile(
#   TARGET <cmake-target>
#   [NAME <archive-stem>]
#   URL <upstream-url>
#   HASH <sha256>
#   [DIR <extract-dir>]
#   [PATCHES_DIR <source-rel-dir>]
#   [OUT_ARCHIVE_VAR <var>]
#   [SOURCE_DIR_VAR <var>]
#   [FILENAME <archive-name>]
# )
#
# Ensures a source archive exists in BENCHAMEL_DISTFILES, extracts it into a
# working directory, and optionally applies a patch series from PATCHES_DIR in
# lexical order. This helper does not configure or build the source tree.
#
# Fetch order:
# 1. Reuse BENCHAMEL_DISTFILES/<archive-name> if its SHA256 matches HASH
# 2. Try BENCHAMEL_DISTFILES_BASE_URL/<archive-name> when that base URL is set
# 3. Fall back to URL
#
# Extraction directory:
# - DIR if provided
# - otherwise <current binary dir>/<NAME>
# - if BENCHAMEL_DISTFILES_IN_SOURCE is ON, relative extraction goes under
#   <current source dir>/.distsrc instead of the build tree
#
# Patches:
# - PATCHES_DIR is relative to the current source directory
# - a symlink to that directory is created inside the extracted tree
# - *.patch and *.patch[0-9] files are applied with patch -p1 in lexical order
#
# Outputs:
# - OUT_ARCHIVE_VAR receives the cached archive path
# - SOURCE_DIR_VAR receives the extracted source directory
function(require_distfile)
  set(options)
  set(one_value_args TARGET NAME URL HASH DIR OUT_ARCHIVE_VAR SOURCE_DIR_VAR
                     FILENAME PATCHES_DIR)
  set(multi_value_args)
  cmake_parse_arguments(DIST "${options}" "${one_value_args}"
                        "${multi_value_args}" ${ARGN})

  if(NOT DIST_NAME AND DIST_TARGET)
    set(DIST_NAME "${DIST_TARGET}")
  endif()

  foreach(required_arg NAME URL HASH)
    if(NOT DIST_${required_arg})
      message(FATAL_ERROR "require_distfile requires ${required_arg}")
    endif()
  endforeach()

  if(NOT BENCHAMEL_DISTFILES)
    message(FATAL_ERROR "require_distfile requires BENCHAMEL_DISTFILES to be set")
  endif()

  if(NOT DIST_OUT_ARCHIVE_VAR)
    set(DIST_OUT_ARCHIVE_VAR DISTFILE_ARCHIVE)
  endif()
  if(NOT DIST_SOURCE_DIR_VAR)
    set(DIST_SOURCE_DIR_VAR DISTFILE_SOURCE_DIR)
  endif()
  if(DIST_FILENAME)
    set(archive_name "${DIST_FILENAME}")
  else()
    set(archive_name "${DIST_NAME}.tar.gz")
  endif()

  set(archive_path "${BENCHAMEL_DISTFILES}/${archive_name}")
  file(MAKE_DIRECTORY "${BENCHAMEL_DISTFILES}")

  set(download_required TRUE)
  if(EXISTS "${archive_path}")
    file(SHA256 "${archive_path}" existing_sha256)
    if(existing_sha256 STREQUAL DIST_HASH)
      set(download_required FALSE)
    else()
      file(REMOVE "${archive_path}")
    endif()
  endif()

  if(download_required)
    set(download_errors)

    if(BENCHAMEL_DISTFILES_BASE_URL)
      string(REGEX REPLACE "/+$" "" base_url
                           "${BENCHAMEL_DISTFILES_BASE_URL}")
      set(base_url_archive "${base_url}/${archive_name}")
      _benchamel_distfile_try_download("${base_url_archive}" "${archive_path}"
                                       "${DIST_HASH}")
      if(BENCHAMEL_DISTFILE_DOWNLOAD_OK)
        set(download_required FALSE)
      else()
        list(APPEND download_errors "${BENCHAMEL_DISTFILE_DOWNLOAD_LOG}")
      endif()
    endif()

    if(download_required)
      _benchamel_distfile_try_download("${DIST_URL}" "${archive_path}"
                                       "${DIST_HASH}")
      if(BENCHAMEL_DISTFILE_DOWNLOAD_OK)
        set(download_required FALSE)
      else()
        list(APPEND download_errors "${BENCHAMEL_DISTFILE_DOWNLOAD_LOG}")
      endif()
    endif()

    if(download_required)
      string(JOIN "\n\n" download_error_message ${download_errors})
      message(FATAL_ERROR "failed to fetch ${DIST_NAME}\n${download_error_message}")
    endif()
  endif()

  if(BENCHAMEL_DISTFILES_IN_SOURCE)
    set(default_extract_parent "${CMAKE_CURRENT_SOURCE_DIR}/.distsrc")
    set(extract_mode "source")
  else()
    set(default_extract_parent "${CMAKE_CURRENT_BINARY_DIR}")
    set(extract_mode "binary")
  endif()

  if(DIST_DIR)
    if(IS_ABSOLUTE "${DIST_DIR}")
      set(source_dir "${DIST_DIR}")
    else()
      set(source_dir "${default_extract_parent}/${DIST_DIR}")
    endif()
  else()
    set(source_dir "${default_extract_parent}/${DIST_NAME}")
  endif()

  get_filename_component(extract_parent "${source_dir}" DIRECTORY)
  get_filename_component(source_dir_name "${source_dir}" NAME)
  set(stamp_file "${source_dir}/.benchamel-distfile-stamp")
  set(
    stamp_signature
    "name=${DIST_NAME}\narchive=${archive_name}\nhash=${DIST_HASH}\nmode=${extract_mode}\nsource_dir=${source_dir}\n"
  )

  set(extract_required TRUE)
  if(EXISTS "${stamp_file}")
    file(READ "${stamp_file}" current_signature)
    if(current_signature STREQUAL stamp_signature)
      set(extract_required FALSE)
    endif()
  endif()

  if(extract_required)
    file(REMOVE_RECURSE "${source_dir}")
    file(MAKE_DIRECTORY "${extract_parent}")

    set(temp_extract_dir "${extract_parent}/.benchamel-extract-${DIST_NAME}")
    file(REMOVE_RECURSE "${temp_extract_dir}")
    file(MAKE_DIRECTORY "${temp_extract_dir}")

    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E tar xf "${archive_path}"
      WORKING_DIRECTORY "${temp_extract_dir}"
      RESULT_VARIABLE extract_rv)
    if(NOT extract_rv EQUAL 0)
      file(REMOVE_RECURSE "${temp_extract_dir}")
      message(FATAL_ERROR "failed to extract ${archive_path}")
    endif()

    if(EXISTS "${temp_extract_dir}/${source_dir_name}")
      set(extracted_dir "${temp_extract_dir}/${source_dir_name}")
    else()
      file(GLOB extracted_children LIST_DIRECTORIES true
           "${temp_extract_dir}/*")
      list(LENGTH extracted_children extracted_children_count)
      if(NOT extracted_children_count EQUAL 1)
        file(REMOVE_RECURSE "${temp_extract_dir}")
        message(
          FATAL_ERROR
            "unable to determine extracted directory for ${DIST_NAME}"
        )
      endif()
      list(GET extracted_children 0 extracted_dir)
    endif()

    file(RENAME "${extracted_dir}" "${source_dir}")
    file(WRITE "${stamp_file}" "${stamp_signature}")
    file(REMOVE_RECURSE "${temp_extract_dir}")
  endif()

  if(DIST_PATCHES_DIR)
    set(patches_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/${DIST_PATCHES_DIR}")
    if(NOT IS_DIRECTORY "${patches_source_dir}")
      message(FATAL_ERROR "PATCHES directory does not exist: ${patches_source_dir}")
    endif()

    set(patches_link_path "${source_dir}/${DIST_PATCHES_DIR}")
    get_filename_component(patches_link_parent "${patches_link_path}" DIRECTORY)
    file(MAKE_DIRECTORY "${patches_link_parent}")
    file(REMOVE_RECURSE "${patches_link_path}")
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E create_symlink "${patches_source_dir}"
              "${patches_link_path}"
      RESULT_VARIABLE patches_symlink_rv)
    if(NOT patches_symlink_rv EQUAL 0)
      message(FATAL_ERROR "failed to create patches symlink at ${patches_link_path}")
    endif()

    string(MAKE_C_IDENTIFIER
           "${CMAKE_CURRENT_SOURCE_DIR}:${DIST_NAME}:${DIST_PATCHES_DIR}"
           patches_stamp_id)
    set(patches_stamp "${source_dir}/.benchamel-${patches_stamp_id}.stamp")
    if(NOT EXISTS "${patches_stamp}")
      file(GLOB patch_files CONFIGURE_DEPENDS RELATIVE "${patches_source_dir}"
           "${patches_source_dir}/*.patch" "${patches_source_dir}/*.patch[0-9]")
      list(SORT patch_files)
      foreach(patch_file IN LISTS patch_files)
        execute_process(
          COMMAND patch -p1 -i "${patches_link_path}/${patch_file}"
          WORKING_DIRECTORY "${source_dir}"
          RESULT_VARIABLE patch_rv)
        if(NOT patch_rv EQUAL 0)
          message(FATAL_ERROR "failed to apply patch ${patch_file} in ${source_dir}")
        endif()
      endforeach()
      file(WRITE "${patches_stamp}" "patched\n")
    endif()
  endif()

  if(DIST_TARGET)
    string(MAKE_C_IDENTIFIER
           "${CMAKE_CURRENT_SOURCE_DIR}:target:${DIST_TARGET}"
           distfile_target_scope_key)
    set_property(GLOBAL PROPERTY
                 "BENCHAMEL_DISTFILE_SOURCE_DIR_${distfile_target_scope_key}"
                 "${source_dir}")
    set_property(GLOBAL PROPERTY
                 "BENCHAMEL_DISTFILE_ARCHIVE_PATH_${distfile_target_scope_key}"
                 "${archive_path}")
  endif()

  string(MAKE_C_IDENTIFIER
         "${CMAKE_CURRENT_SOURCE_DIR}:name:${DIST_NAME}"
         distfile_name_scope_key)
  set_property(GLOBAL PROPERTY
               "BENCHAMEL_DISTFILE_SOURCE_DIR_${distfile_name_scope_key}"
               "${source_dir}")
  set_property(GLOBAL PROPERTY
               "BENCHAMEL_DISTFILE_ARCHIVE_PATH_${distfile_name_scope_key}"
               "${archive_path}")

  set(${DIST_OUT_ARCHIVE_VAR}
      "${archive_path}"
      PARENT_SCOPE)
  set(${DIST_SOURCE_DIR_VAR}
      "${source_dir}"
      PARENT_SCOPE)
endfunction()
