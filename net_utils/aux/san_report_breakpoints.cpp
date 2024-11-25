#include <iostream>


extern "C" {


void __ubsan_on_report() {
  std::cout << "Encountered an undefined behavior sanitizer error" << std::endl;
}

void __asan_on_error() {
  std::cout << "Encountered an address sanitizer error" << std::endl;
}

void __tsan_on_report() {
  std::cout << "Encountered a thread sanitizer error" << std::endl;
}

void __msan_on_report() {
  std::cout << "Encountered a memory sanitizer error" << std::endl;
}

void __lsan_on_report() {
  std::cout << "Encountered a leak sanitizer error" << std::endl;
}


}// extern "C"