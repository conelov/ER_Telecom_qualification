include_guard(GLOBAL)


function(download_file url to)
  set(options OVERWRITE SHOW_PROGRESS)
  set(args)
  set(comargs)
  cmake_parse_arguments(PARSE_ARGV 2 ARG "${options}" "${args}" "${comargs}")

  cmake_path(SET to NORMALIZE "${to}")
  if(EXISTS "${to}" AND NOT ARG_OVERWRITE)
    return()
  endif()

  if(ARG_SHOW_PROGRESS)
    set(show_progress_cmd SHOW_PROGRESS)
  endif()

  get_filename_component(file "${to}" NAME)
  message(STATUS "Try download ${file} from ${url}")
  cmake_language(EVAL CODE "file(DOWNLOAD ${url} \"${to}\" \
    STATUS check \
    ${show_progress_cmd} \
  )")
  list(GET check 0 check)
  if(NOT ${check} EQUAL 0)
    message(FATAL_ERROR "Failed to download ${file}")
  endif()
endfunction()


function(auto_fetch)
  set(file CPM.cmake)
  cmake_path(SET file_path NORMALIZE "${CMAKE_BINARY_DIR}/${file}")
  download_file(https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake "${file_path}")

  option(CPM_USE_NAMED_CACHE_DIRECTORIES "" ON)
  option(CPM_USE_LOCAL_PACKAGES "" ON)
  include("${file_path}")
  CPMAddPackage(${ARGN}) # https://github.com/cpm-cmake/CPM.cmake
endfunction()


function(target_common target)
  target_include_directories(${target} PRIVATE
    "${PROJECT_SOURCE_DIR}"
  )
  target_compile_options(${target} PRIVATE
    $<$<OR:$<COMPILE_LANG_AND_ID:CXX,Clang>,$<COMPILE_LANG_AND_ID:CXX,GNU>>:-Wall>
    $<$<CONFIG:DEBUG>:-v>
    $<$<AND:$<COMPILE_LANG_AND_ID:CXX,GNU>,$<CONFIG:DEBUG>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,10.0.0>>:-fanalyzer>
  )
endfunction()


function(aux_common target)
  target_common(${name})
  target_sources(${target} PRIVATE
    "${PROJECT_SOURCE_DIR}/net_utils/aux/MultiThreadedFixture.hpp"
  )
endfunction()