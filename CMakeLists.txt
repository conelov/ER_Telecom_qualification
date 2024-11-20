cmake_minimum_required(VERSION 3.24)
project(ER_Telecom_qualification LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(BUILD_TESTING "Enable testing" ${PROJECT_IS_TOP_LEVEL})
set(GTEST_VERSION 1.15.2 CACHE STRING "")
set(TESTS_SANITIZE "address;thread" CACHE STRING "")


add_subdirectory(net_utils)
