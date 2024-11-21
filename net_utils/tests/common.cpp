#include <gtest/gtest.h>


extern "C" {
  void __ubsan_on_report() {
    ADD_FAILURE() << "Encountered an undefined behavior sanitizer error";
  }
  void __asan_on_error() {
    ADD_FAILURE() << "Encountered an address sanitizer error";
  }
  void __tsan_on_report() {
    ADD_FAILURE() << "Encountered a thread sanitizer error";
  }
  void __msan_on_report() {
    ADD_FAILURE() << "Encountered a memory sanitizer error";
  }
  void __lsan_on_report() {
    ADD_FAILURE() << "Encountered a leak sanitizer error";
  }
}  // extern "C"