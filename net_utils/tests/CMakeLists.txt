include("${PROJECT_SOURCE_DIR}/cmake/utils.cmake")


function(test_common out_var suffix)
  # https://github.com/cpm-cmake/CPM.cmake/tree/master/examples/gtest
  auto_fetch(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    VERSION ${${PROJECT_NAME_UP_CASE}_GTEST_VERSION}
    OPTIONS "INSTALL_GTEST OFF"
  )

  set(name ${PROJECT_NAME}-test-${suffix})
  add_executable(${name})
  target_common(${name})
  aux_common(${name})
  collect_targets(${name} test)

  add_test(NAME ${name} COMMAND "$<TARGET_FILE:${name}>")

  target_link_libraries(${name} PRIVATE
    GTest::gmock
    GTest::gmock_main
  )

  set(${out_var} ${name} PARENT_SCOPE)
endfunction()


macro(test_once name)
  san_common(tests ${name} test_common)
  foreach(i IN LISTS tests)
    target_sources(${i} PRIVATE
      ${name}.cpp
    )
  endforeach()
endmacro()

foreach(i LruStorage PriorityMutex RcuStorage IterativeAverage)
  test_once(${i})
endforeach()


san_common(tests DnsCache test_common)
foreach(i IN LISTS tests)
  target_sources(${i} PRIVATE
    DnsCache.cpp
  )
  target_link_libraries(${i} PRIVATE
    ${PROJECT_NAME}_lib
  )
endforeach()
