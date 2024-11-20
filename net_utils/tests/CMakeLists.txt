set(name ER_Telecom_net_utils-tests)

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