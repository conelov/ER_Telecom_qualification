#pragma once

#include <algorithm>
#include <cstddef>


namespace nut {


template<typename T_>
class IterativeAverage final {
public:
  using value_type = T_;

  struct MinMax final {
    value_type min;
    value_type max;
  };

public:
  IterativeAverage(value_type init = {}) noexcept {
    reset(init);
  }


  value_type add(value_type const value) noexcept {
    minmax_.min = std::min(minmax_.min, value);
    minmax_.max = std::max(minmax_.max, value);
    return avr_ += (value - avr_) / ++count_;
  }


  IterativeAverage& operator+=(value_type value) noexcept {
    add(value);
    return *this;
  }


  [[nodiscard]] std::size_t count() const noexcept {
    return count_;
  }


  [[nodiscard]] value_type average() const noexcept {
    return avr_;
  }


  [[nodiscard]] operator value_type() const noexcept {
    return average();
  }


  void reset(value_type init = {}) noexcept {
    minmax_.min = minmax_.max = avr_ = init;
    count_                           = 0;
  }


  [[nodiscard]] MinMax min_max() const noexcept {
    return minmax_;
  }

private:
  value_type      avr_;
  std::size_t count_;
  MinMax      minmax_;
};


}// namespace nut