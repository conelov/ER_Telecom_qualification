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


function(cpu_count out_var)
  set(cpu_count_var ${PROJECT_NAME}-cpu_count)
  if(NOT DEFINED CACHE{${cpu_count_var}})
    if(UNIX)
      execute_process(COMMAND sh -c nproc
        OUTPUT_VARIABLE c
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    else()
      set(c 4)
    endif()
    set(${cpu_count_var} ${c} CACHE INTERNAL "")
  endif()
  set(${out_var} ${${cpu_count_var}} PARENT_SCOPE)
endfunction()


function(auto_fetch)
  set(cpm_folder_var ${CMAKE_CURRENT_FUNCTION}_cpm_folder)
  if(NOT DEFINED CACHE{${cpm_folder_var}})
    set(dest "${CPM_SOURCE_CACHE}")
    if("${dest}" STREQUAL "")
      set(dest "$ENV{CPM_SOURCE_CACHE}")
    endif()
    if("${dest}" STREQUAL "")
      set(dest "${CMAKE_BINARY_DIR}/cpm_repo")
      set(CPM_SOURCE_CACHE "${dest}" CACHE STRING "")
    endif()
    set(${cpm_folder_var} "${dest}" CACHE INTERNAL "")
  endif()

  set(file CPM.cmake)
  cmake_path(SET cpm_file NORMALIZE "${${cpm_folder_var}}/${file}")
  download_file(https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake "${cpm_file}")

  option(CPM_USE_NAMED_CACHE_DIRECTORIES "" ON)
  option(CPM_USE_LOCAL_PACKAGES "" ON)
  include("${cpm_file}")
  CPMAddPackage(${ARGN}) # https://github.com/cpm-cmake/CPM.cmake
endfunction()


function(target_common target)
  target_include_directories(${target} PRIVATE
    "${PROJECT_SOURCE_DIR}"
  )
  target_compile_options(${target} PRIVATE
    $<$<OR:$<COMPILE_LANG_AND_ID:CXX,Clang>,$<COMPILE_LANG_AND_ID:CXX,GNU>>:-Wall>
    $<$<CONFIG:DEBUG>:-v>
    #    $<$<AND:$<COMPILE_LANG_AND_ID:CXX,GNU>,$<CONFIG:DEBUG>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,10.0.0>>:-fanalyzer>
  )
  target_link_options(${target} PRIVATE
    $<$<CONFIG:DEBUG>:-v>
  )
  target_compile_definitions(${target} PUBLIC
    $<$<BOOL:${BUILD_TESTING}>:NUT_TESTING>
    CXX_COMPILER_ID_${CMAKE_CXX_COMPILER_ID}
  )
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0.0")
    target_compile_definitions(${target} PUBLIC
      ${PROJECT_NAME_UP_CASE}_CXX_GNU_L_11
    )
  endif()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    target_compile_definitions(${target} PUBLIC
      NUT_ARCH_X86
    )
  endif()
endfunction()


function(aux_common target)
  target_common(${name})
  list(APPEND sources
    IterationRate.hpp
    MultiThreadedFixture.hpp
    MultiThreadedRWFixture.hpp
    MultiThreadedRWValuedFixture.hpp
    PriorityMutexFixture.hpp
    RcuStorageFixture.hpp
    DnsCacheFixture.hpp
    san_report_breakpoints.cpp
  )
  list(TRANSFORM sources PREPEND "${PROJECT_SOURCE_DIR}/net_utils/aux/")
  target_sources(${target} PRIVATE
    ${sources}
  )

  cpu_count(c)
  target_compile_definitions(${name} PRIVATE
    ${PROJECT_NAME_UP_CASE}_CPU_COUNT=${c}
  )
endfunction()


function(san_common out_var suffix fn_gen)
  unset(${out_var})
  macro(asan)
    target_compile_options(${name} PRIVATE
      -fsanitize=address
      -fno-common
      -fno-omit-frame-pointer
      -fsanitize-address-use-after-scope
    )
    target_link_options(${name} PRIVATE
      -fsanitize=address
    )
  endmacro()

  macro(tsan)
    target_compile_options(${name} PUBLIC
      -fsanitize=thread
    )
    target_link_options(${name} PUBLIC
      -fsanitize=thread
    )
  endmacro()

  macro(msan)
    target_compile_options(${name} PRIVATE
      -fsanitize=memory
      -fsanitize-memory-track-origins
      -fno-omit-frame-pointer
      -fno-optimize-sibling-calls
    )
    target_link_options(${name} PRIVATE
      -fsanitize=memory
    )
  endmacro()

  macro(usan)
    target_compile_options(${name} PRIVATE
      -fsanitize=undefined
      $<$<COMPILE_LANG_AND_ID:CXX,Clang>:
      -fsanitize=integer
      -fsanitize=nullability
      >
    )
    target_link_options(${name} PRIVATE
      -fsanitize=undefined
      -lubsan
      $<$<COMPILE_LANG_AND_ID:CXX,Clang>:
      -fsanitize=integer
      -fsanitize=nullability
      >
    )
  endmacro()

  macro(lsan)
    target_compile_options(${name} PUBLIC
      -fsanitize=leak
    )
    target_link_options(${name} PUBLIC
      -fsanitize=leak
    )
  endmacro()

  foreach(i IN LISTS ${PROJECT_NAME_UP_CASE}_SANITIZERS)
    if("${i}" STREQUAL "address")
      cmake_language(CALL ${fn_gen} name "${suffix}-asan")
      asan()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "mem")
      if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(STATUS "Memory sanitizer disabled.")

      else()
        cmake_language(CALL ${fn_gen} name "${suffix}-msan")
        msan()
      endif()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "thread")
      cmake_language(CALL ${fn_gen} name "${suffix}-tsan")
      tsan()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "leak")
      cmake_language(CALL ${fn_gen} name "${suffix}-lsan")
      lsan()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "ub")
      cmake_language(CALL ${fn_gen} name "${suffix}-usan")
      usan()
      list(APPEND ${out_var} ${name})
    endif()
  endforeach()

  cmake_language(CALL ${fn_gen} name "${suffix}")
  list(APPEND ${out_var} ${name})

  set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()


function(collect_targets target scope)
  set(scope_var ${PROJECT_NAME}_${CMAKE_CURRENT_FUNCTION}_${scope})

  set(callback_fn_var ${PROJECT_NAME}_${CMAKE_CURRENT_FUNCTION}_${scope}_callback)
  set(build_all_var ${PROJECT_NAME}_build_all_${scope})
  if(NOT TARGET ${build_all_var})
    add_custom_target(${build_all_var})

    function(${callback_fn_var} scope scope_var build_all_var)
      unset(args)
      foreach(i IN LISTS ${scope_var})
        set(args "${args} COMMAND \"$<TARGET_FILE:${i}>\"")
      endforeach()
      set(run_all_var ${PROJECT_NAME}_run_all_${scope})
      cmake_language(EVAL CODE "add_custom_target(${run_all_var} USES_TERMINAL ${args})")
      add_dependencies(${run_all_var} ${build_all_var})
    endfunction()
    cmake_language(EVAL CODE "cmake_language(DEFER DIRECTORY \"${PROJECT_SOURCE_DIR}\" CALL ${callback_fn_var} ${scope} ${scope_var} ${build_all_var})")
  endif()

  add_dependencies(${build_all_var} ${target})
  list(APPEND ${scope_var} ${target})
  set(${scope_var} "${${scope_var}}" CACHE INTERNAL "")
endfunction()