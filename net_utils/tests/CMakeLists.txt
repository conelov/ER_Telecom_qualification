function(test_common suffix out_var)
  set(name ER_Telecom_net_utils-tests${suffix})

  add_executable(${name}
    tests_general.cpp
    RcuStorage.cpp
  )

  include("${PROJECT_SOURCE_DIR}/utils.cmake")
  # https://github.com/cpm-cmake/CPM.cmake/tree/master/examples/gtest
  auto_fetch(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    VERSION ${GTEST_VERSION}
    OPTIONS "INSTALL_GTEST OFF"
  )

  target_link_libraries(${name} PRIVATE
    GTest::gmock
    GTest::gmock_main
    ER_Telecom_net_utils
  )

  target_common(${name})
  set(${out_var} ${name} PARENT_SCOPE)
endfunction()

test_common("" name)


foreach(i IN LISTS TESTS_SANITIZE)
  if("${i}" STREQUAL "address")
    test_common("-san_address" name)
    target_compile_options(${name} PRIVATE
      -fsanitize=address
      -fno-omit-frame-pointer
      -fsanitize-recover
    )
    target_link_options(${name} PRIVATE
      -fsanitize=address
    )
  endif()


  if("${i}" STREQUAL "thread")
    test_common("-san_thread" name)
    target_compile_options(${name} PRIVATE
      -fsanitize=thread
    )
    target_link_options(${name} PRIVATE
      -fsanitize=thread
    )
  endif()
endforeach()
