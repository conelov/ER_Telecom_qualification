include("${PROJECT_SOURCE_DIR}/utils.cmake")


test_common(name RcuStorage)
test_make_sans(list_sans RcuStorage)
list(APPEND list_sans ${name})
foreach(i IN LISTS list_sans)
  target_sources(${i} PRIVATE
    RcuStorage.cpp
  )
endforeach()



test_common(name DnsCache-cache)
test_make_sans(list_sans DnsCache-cache)
list(APPEND list_sans ${name})
foreach(i IN LISTS list_sans)
  target_sources(${i} PRIVATE
    DnsCache.cpp
  )
  target_link_libraries(${i} PRIVATE
    ER_Telecom_net_utils
  )
endforeach()
