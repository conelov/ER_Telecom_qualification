include("${PROJECT_SOURCE_DIR}/utils.cmake")


function(bench_common out_var suffix)
  auto_fetch(
    NAME benchmark
    GITHUB_REPOSITORY google/benchmark
    VERSION ${GBENCH_VERSION}
    OPTIONS "BENCHMARK_ENABLE_TESTING OFF"
  )

  set(name ${PROJECT_NAME}-bench-${suffix})
  add_executable(${name})
  target_common(${name})
  aux_common(${name})
  collect_targets(${name} bench)


  target_sources(${name} PRIVATE
    main.cpp
    common.hpp
  )

  target_link_libraries(${name} PRIVATE
    benchmark::benchmark
  )

  set(${out_var} ${name} PARENT_SCOPE)
endfunction()


macro(bench_once name)
  if(GBENCH_SAN)
    san_common(ts ${name} bench_common)
  else()
    bench_common(ts ${name})
  endif()
  foreach(i IN LISTS ts)
    target_sources(${i} PRIVATE
      ${name}.cpp
    )
    target_link_libraries(${i} PRIVATE
      ER_Telecom_net_utils
    )
  endforeach()
endmacro()

foreach(i DnsCacheImpl PriorityMutex RcuStorage)
  bench_once(${i})
endforeach()
