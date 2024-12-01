set(name ${PROJECT_NAME}_lib)

add_library(${name}
  utils.hpp
  IterativeAverage.hpp
  Singleton.hpp
  ConcurrencyUtils.hpp
  LruStorage.hpp
  LruStorageLockFree.hpp
  RcuStorage.hpp
  RcuLruStorage.hpp
  PriorityMutex.hpp
  DnsCache.hpp
  DnsCacheImpl.hpp src/DnsCacheImpl.cpp
)

include("${PROJECT_SOURCE_DIR}/cmake/utils.cmake")
target_common(${name})

target_compile_definitions(${name} PUBLIC
  ${PROJECT_NAME_UP_CASE}_DNS_CACHE_REC_LIMIT=${${PROJECT_NAME_UP_CASE}_DNS_CACHE_REC_LIMIT}
)


if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(${PROJECT_NAME_UP_CASE}_RESEARCH)
  add_subdirectory(bench)
endif()