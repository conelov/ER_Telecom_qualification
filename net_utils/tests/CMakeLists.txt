include("${PROJECT_SOURCE_DIR}/utils.cmake")


add_custom_target(build_tests)

function(test_common_base out_var suffix)
  # https://github.com/cpm-cmake/CPM.cmake/tree/master/examples/gtest
  auto_fetch(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    VERSION ${GTEST_VERSION}
    OPTIONS "INSTALL_GTEST OFF"
  )

  set(name ${PROJECT_NAME}-test-${suffix})
  add_executable(${name}
    common.cpp
  )
  aux_common(${name})

  add_dependencies(build_tests ${name})
  add_test(NAME ${name} COMMAND "$<TARGET_FILE:${name}>")

  target_link_libraries(${name} PRIVATE
    GTest::gtest
    GTest::gtest_main
  )

  set(${out_var} ${name} PARENT_SCOPE)
endfunction()


function(test_common out_var suffix)
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

  foreach(i IN LISTS TESTS_SANITIZE)
    if("${i}" STREQUAL "address")
      test_common_base(name "${suffix}-asan")
      asan()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "mem")
      if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(STATUS "Memory sanitizer disabled.")

      else()
        test_common_base(name "${suffix}-msan")
        msan()
      endif()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "thread")
      test_common_base(name "${suffix}-tsan")
      tsan()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "leak")
      test_common_base(name "${suffix}-lsan")
      lsan()
      list(APPEND ${out_var} ${name})
    endif()

    if("${i}" STREQUAL "ub")
      test_common_base(name "${suffix}-usan")
      usan()
      list(APPEND ${out_var} ${name})
    endif()
  endforeach()

  test_common_base(name "${suffix}")
  list(APPEND ${out_var} ${name})

  set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()


test_common(tests RcuStorage)
foreach(i IN LISTS tests)
  target_sources(${i} PRIVATE
    RcuStorage.cpp
  )
endforeach()


test_common(tests DnsCache)
foreach(i IN LISTS tests)
  target_sources(${i} PRIVATE
    DnsCache.cpp
  )
  target_link_libraries(${i} PRIVATE
    ER_Telecom_net_utils
  )
endforeach()


test_common(tests SpinlockRW)
foreach(i IN LISTS tests)
  target_sources(${i} PRIVATE
    SpinlockRW.cpp
  )
endforeach()
