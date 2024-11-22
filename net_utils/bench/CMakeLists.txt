function(bench_common out_var suffix)
  include("${PROJECT_SOURCE_DIR}/utils.cmake")
  auto_fetch(
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

  target_compile_definitions(${name} PRIVATE
    CPU_COUNT=${${cpu_count_var}}
  )

  aux_common(${name})

  set(${out_var} ${name} PARENT_SCOPE)
endfunction()


bench_common(name DnsCache)
target_sources(${name} PRIVATE
  DnsCacheImpl.cpp
)
target_link_libraries(${name} PRIVATE
  ER_Telecom_net_utils
)
