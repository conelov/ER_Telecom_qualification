#pragma once

#include <chrono>

#include <net_utils/utils.hpp>


namespace nut::aux {


class IterationRate final {
public:
  using Clock = std::chrono::steady_clock;

private:
  static std::int64_t time() noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
  }

public:
  [[nodiscard]] IterativeAverage current() const noexcept {
    return ir_;
  }


  [[nodiscard]] double operator*() const noexcept {
    return ir_.average();
  }


  [[nodiscard]] operator double() const noexcept {
    return **this;
  }


  double cut() noexcept {
    return ir_.add(time());
  }


  void operator++() noexcept {
    cut();
  }


  void operator++(int) noexcept {
    cut();
  }


  void reset() noexcept {
    ir_ = time();
  }

private:
  IterativeAverage ir_ = time();
};


}// namespace nut::aux