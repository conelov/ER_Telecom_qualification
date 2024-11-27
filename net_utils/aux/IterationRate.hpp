#pragma once

#include <chrono>

#include <net_utils/IterativeAverage.hpp>


namespace nut::aux {


template<typename Ratio_ = std::nano>
class IterationRate final {
public:
  using Clock    = std::chrono::steady_clock;
  using Ratio    = Ratio_;
  using Rep      = double;
  using Duration = std::chrono::duration<Rep, Ratio>;
  using Average  = IterativeAverage<Duration>;

public:
  std::size_t resolution = 1;

public:
  IterationRate() {// NOLINT(*-pro-type-member-init)
    reset();
  }


  [[nodiscard]] Average const& current() const noexcept {
    return ir_;
  }


  [[nodiscard]] Average const& operator*() const noexcept {
    return current();
  }


  [[nodiscard]] Average const* operator->() const noexcept {
    return &ir_;
  }


  [[nodiscard]] operator Duration() const noexcept {
    return ir_.average();
  }


  void operator++() noexcept {
    iterate();
  }


  void operator++(int) noexcept {
    iterate();
  }


  void iterate() noexcept {
    if (++iteration_counter_ < resolution) {
      return;
    }
    cut(iteration_counter_);
    iteration_counter_ = 0;
  }


  void cut() noexcept {
    cut(iteration_counter_);
  }


  void reset() noexcept {
    ir_.reset();
    time_prev_         = Clock::now();
    iteration_counter_ = 0;
  }


  Average release() noexcept {
    auto const out = ir_;
    reset();
    return out;
  }

private:
  void cut(std::size_t iters) {
    if (iters == 0) {
      return;
    }
    auto const start = std::exchange(time_prev_, Clock::now());
    ir_ += static_cast<Rep>(std::chrono::duration_cast<std::chrono::duration<std::uint64_t>>(time_prev_ - start).count())
      / static_cast<Rep>(iters);
  }

private:
  Average           ir_;
  Clock::time_point time_prev_;
  std::size_t       iteration_counter_;
};


}// namespace nut::aux