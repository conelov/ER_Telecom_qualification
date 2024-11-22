include("${PROJECT_SOURCE_DIR}/utils.cmake")


function(bench_common out_var suffix)
  CPMAddPackage(
    NAME benchmark
    GITHUB_REPOSITORY google/benchmark
    VERSION ${GBENCH_VERSION}
    OPTIONS "BENCHMARK_ENABLE_TESTING OFF"
  )

  set(name ${PROJECT_NAME}-bench-${suffix})
  add_executable(${name})
  target_common(${name})

  target_link_libraries(${name} PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
  )

  aux_common(${name})

  set(${out_var} ${name} PARENT_SCOPE)
endfunction()


bench_common(name DnsCache)
target_sources(${name} PRIVATE
  DnsCacheImpl.cpp
  "${PROJECT_SOURCE_DIR}/net_utils/aux/DnsCacheFixture.hpp"
)
target_link_libraries(${name} PRIVATE
  ER_Telecom_net_utils
)
