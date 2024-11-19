set(name ER_Telecom_net_utils)

add_library(${name}
  DnsCacheInterface.hpp
  DnsCacheInterface.hpp src/DnsCache.cpp
  Singleton.hpp
  utils.hpp
)

include("${PROJECT_SOURCE_DIR}/utils.cmake")
target_common(${name})

target_compile_definitions(${name} PRIVATE
  DNS_CACHE_REC_LIMIT=${DNS_CACHE_REC_LIMIT}
)


if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
