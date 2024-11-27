#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <limits>


namespace nut {


template<typename T_ = double>
class IterativeAverage final {
public:
  using value_type = T_;
  static_assert(std::is_floating_point_v<value_type>);

  struct MinMax final {
    value_type min;
    value_type max;
  };

public:
  IterativeAverage() noexcept {
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
    assert(count_ > 0);
    return avr_;
  }


  [[nodiscard]] operator value_type() const noexcept {
    return average();
  }


  void reset() noexcept {
    avr_        = 0;
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