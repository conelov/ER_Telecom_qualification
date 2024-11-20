set(DNS_CACHE_REC_LIMIT 10 CACHE STRING "")
set(DNS_CACHE_CLEANUP_LIMIT 12 CACHE STRING "")


set(name ER_Telecom_net_utils)

add_library(${name}
  DnsCacheInterface.hpp
  DnsCacheInterface.hpp src/DnsCache.cpp
  Singleton.hpp
  utils.hpp
  Flags.hpp
  RcuStorage.hpp
)

include("${PROJECT_SOURCE_DIR}/utils.cmake")
target_common(${name})

target_compile_definitions(${name} PRIVATE
  DNS_CACHE_REC_LIMIT=${DNS_CACHE_REC_LIMIT}
  DNS_CACHE_CLEANUP_LIMIT=${DNS_CACHE_CLEANUP_LIMIT}
)


if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
