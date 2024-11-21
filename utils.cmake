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
  set(dest "${CPM_SOURCE_CACHE}")
  if("${dest}" STREQUAL "")
    set(dest "$ENV{CPM_SOURCE_CACHE}")
  endif()
  if("${dest}" STREQUAL "")
    set(dest "${CMAKE_BINARY_DIR}/cpm_repo")
  endif()

  set(file CPM.cmake)
  cmake_path(SET cpm_file NORMALIZE "${dest}/${file}")
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
  )
endfunction()


add_custom_target(build_tests)

function(test_common out_var suffix)
  set(name ER_Telecom_net_utils-test-${suffix})
  add_executable(${name})
  add_dependencies(build_tests ${name})

  add_test(NAME ${name} COMMAND "$<TARGET_FILE:${name}>")

  include("${PROJECT_SOURCE_DIR}/utils.cmake")
  # https://github.com/cpm-cmake/CPM.cmake/tree/master/examples/gtest
  auto_fetch(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    VERSION ${GTEST_VERSION}
    OPTIONS "INSTALL_GTEST OFF"
  )

  target_link_libraries(${name} PRIVATE
    GTest::gtest
    GTest::gtest_main
  )

  target_compile_options(${name} PRIVATE
    $<$<AND:$<COMPILE_LANG_AND_ID:CXX,GNU>,$<CONFIG:DEBUG>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,10.0.0>>:-fanalyzer>
  )

  target_include_directories(${name} PRIVATE
    "${PROJECT_SOURCE_DIR}/net_utils/tests"
  )

  target_sources(${name} PRIVATE
    "${PROJECT_SOURCE_DIR}/net_utils/tests/MultiThreadedFixture.hpp"
  )

  target_common(${name})
  set(${out_var} ${name} PARENT_SCOPE)
endfunction()


function(test_make_sans out_var suffix)
  foreach(i IN LISTS TESTS_SANITIZE)
    if("${i}" STREQUAL "address")
      test_common(name "${suffix}-san_address")
      target_compile_options(${name} PRIVATE
        -fsanitize=address
        -fno-common
        -fno-omit-frame-pointer
        -fsanitize-address-use-after-scope
      )
      target_link_options(${name} PRIVATE
        -fsanitize=address
      )
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "mem")
      if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(STATUS "Memory sanitizer disabled.")

      else()
        test_common(name "${suffix}-san_mem")
        target_compile_options(${name} PRIVATE
          -fsanitize=memory
          -fsanitize-memory-track-origins
          -fno-omit-frame-pointer
          -fno-optimize-sibling-calls
        )
        target_link_options(${name} PRIVATE
          -fsanitize=memory
        )
      endif()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "thread")
      test_common(name "${suffix}-san_thread")
      target_compile_options(${name} PUBLIC
        -fsanitize=thread
      )
      target_link_options(${name} PUBLIC
        -fsanitize=thread
      )
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "leak")
      test_common(name "${suffix}-san_leak")
      target_compile_options(${name} PUBLIC
        -fsanitize=leak
      )
      target_link_options(${name} PUBLIC
        -fsanitize=leak
      )
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "ub")
      test_common(name "${suffix}-san-ub")
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
      list(APPEND ${out_var} ${name})
    endif()
  endforeach()

  set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()