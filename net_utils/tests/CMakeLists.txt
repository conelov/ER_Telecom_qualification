function(test_RcuStorage_common out_var suffix)
  test_common(name RcuStorage${suffix})
  target_sources(${name} PRIVATE
    RcuStorage.cpp
  )
  target_link_libraries(${name} PRIVATE
    GTest::gtest_main
  )
  set(${out_var} ${name} PARENT_SCOPE)
endfunction()

test_RcuStorage_common(name "")

foreach(i IN LISTS TESTS_SANITIZE)
  if("${i}" STREQUAL "address")
    test_RcuStorage_common(name "-san_address")
    target_compile_options(${name} PRIVATE
      -fsanitize=address
      -fno-common
      -fno-omit-frame-pointer
      -fsanitize-address-use-after-scope
    )
    target_link_options(${name} PRIVATE
      -fsanitize=address
    )
  endif()

  if("${i}" STREQUAL "mem")
    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      message(STATUS "Memory sanitizer disabled.")

    else()
      test_RcuStorage_common(name "-san_mem")
      target_compile_options(${name} PRIVATE
        -fsanitize=memory
        -fsanitize-memory-track-origins
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
      )
      target_link_options(${name} PRIVATE
        -fsanitize=memory
      )
    endif()
  endif()

  if("${i}" STREQUAL "thread")
    test_RcuStorage_common(name "-san_thread")
    target_compile_options(${name} PUBLIC
      -fsanitize=thread
    )
    target_link_options(${name} PUBLIC
      -fsanitize=thread
    )
  endif()

  if("${i}" STREQUAL "leak")
    test_RcuStorage_common(name "-san_leak")
    target_compile_options(${name} PUBLIC
      -fsanitize=leak
    )
    target_link_options(${name} PUBLIC
      -fsanitize=leak
    )
  endif()

  if("${i}" STREQUAL "ub")
    test_RcuStorage_common(name "-san-ub")
    target_compile_options(${name} PRIVATE
      -fsanitize=undefined
      $<$<COMPILE_LANG_AND_ID:CXX,Clang>:
      -fsanitize=integer
      -fsanitize=nullability
      >
    )
    target_link_options(${name} PRIVATE
      -fsanitize=undefined
      -lubsan
      $<$<COMPILE_LANG_AND_ID:CXX,Clang>:
      -fsanitize=integer
      -fsanitize=nullability
      >
    )
  endif()
endforeach()


function(test_DnsCache_common out_var cache_size)
  test_common(name DnsCache-cache_${cache_size})
  target_sources(${name} PRIVATE
    DnsCache.cpp
  )
  target_link_libraries(${name} PRIVATE
    ER_Telecom_net_utils
  )
  target_compile_definitions(${name} PRIVATE
    CACHE_LIMIT=${cache_size}
  )
  set(${out_var} ${name} PARENT_SCOPE)
endfunction()

foreach(i 3 100000 10000000)
  test_DnsCache_common(name ${i})
endforeach()
