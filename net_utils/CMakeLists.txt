set(DNS_CACHE_REC_LIMIT 10 CACHE STRING "")


set(name ER_Telecom_net_utils)

add_library(${name}
  utils.hpp
  Singleton.hpp
  LruStorage.hpp
  PriorityMutex.hpp
  DnsCache.hpp
  DnsCacheImpl.hpp src/DnsCacheImpl.cpp
)

include("${PROJECT_SOURCE_DIR}/utils.cmake")
target_common(${name})

target_compile_definitions(${name} PUBLIC
  DNS_CACHE_REC_LIMIT=${DNS_CACHE_REC_LIMIT}
  $<$<BOOL:${BUILD_GBENCH}>:BUILD_GBENCH>
)


if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(BUILD_GBENCH)
  add_subdirectory(bench)
endif()