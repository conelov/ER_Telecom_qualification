cmake_minimum_required(VERSION 3.24)
project(net_utils LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(BUILD_TESTING "Enable testing" ${PROJECT_IS_TOP_LEVEL})
set(GTEST_VERSION 1.15.2 CACHE STRING "")
set(TESTS_SANITIZE "address;thread;ub;leak;mem" CACHE STRING "")
option(BUILD_GBENCH "Enable benchmark" ${PROJECT_IS_TOP_LEVEL})
set(GBENCH_VERSION 1.9.0 CACHE STRING "")
option(GBENCH_SAN "Use sanitizers for benchmarking" OFF)
set(DNS_CACHE_REC_LIMIT 1000 CACHE STRING "")


if(BUILD_TESTING AND NOT "${TESTS_SANITIZE}" STREQUAL "")
  set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
  enable_testing()
endif()


add_subdirectory(net_utils)
