set(name ER_Telecom_net_utils)

add_library(${name}
  DnsCacheInterface.hpp
  DnsCacheInterface.hpp src/DnsCache.cpp
)

include("${PROJECT_SOURCE_DIR}/utils.cmake")
target_common(${name})


if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
