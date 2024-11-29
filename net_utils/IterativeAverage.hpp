#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>


namespace nut {


template<typename T_ = double>
class IterativeAverage final {
public:
  using value_type = T_;

  struct MinMax final {
    value_type min;
    value_type max;
  };

public:
  IterativeAverage() noexcept {// NOLINT(*-pro-type-member-init)
    reset();
  }


  IterativeAverage& add(value_type const in) noexcept {
    minmax_.min = std::min(minmax_.min, in);
    minmax_.max = std::max(minmax_.max, in);
    avr_ += (in - avr_) / ++count_;
    return *this;
  }


  IterativeAverage& operator+=(value_type value) noexcept {
    return add(value);
  }


  [[nodiscard]] std::size_t count() const noexcept {
    return count_;
  }


  [[nodiscard]] value_type average() const noexcept {
    assert(!empty());
    return avr_;
  }


  [[nodiscard]] operator value_type() const noexcept {
    return average();
  }


  [[nodiscard]] bool empty() const noexcept {
    return count_ == 0;
  }


  void reset() noexcept {
    avr_        = {};
    count_      = 0;
    minmax_.min = std::numeric_limits<value_type>::max();
    minmax_.max = std::numeric_limits<value_type>::min();
  }


  [[nodiscard]] MinMax min_max() const noexcept {
    return minmax_;
  }

private:
  value_type  avr_;
  std::size_t count_;
  MinMax      minmax_;
};


}// namespace nut