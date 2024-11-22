set(DNS_CACHE_REC_LIMIT 10 CACHE STRING "")


set(name ER_Telecom_net_utils)

add_library(${name}
  DnsCache.hpp src/DnsCache.cpp
  Singleton.hpp
  utils.hpp
  RcuStorage.hpp
)

include("${PROJECT_SOURCE_DIR}/utils.cmake")
target_common(${name})

target_compile_definitions(${name} PUBLIC
  DNS_CACHE_REC_LIMIT=${DNS_CACHE_REC_LIMIT}
)


if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(BUILD_GBENCH)
  add_subdirectory(bench)
endif()