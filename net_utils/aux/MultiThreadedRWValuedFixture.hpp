#pragma once

#include <functional>
#include <optional>

#include <net_utils/aux/MultiThreadedRWFixture.hpp>


namespace nut::aux {


template<typename T_>
class MultiThreadedRWValuedFixture : public MultiThreadedRWFixture {
public:
  using value_type = T_;
  using ValueOpt   = std::optional<value_type>;
  using ValueCtor  = std::function<void(ValueOpt&)>;

public:
  ValueCtor value_ctor;

private:
  void post_stop() override {
    value_.reset();
  }


  void pre_start() override {
    assert(!value_.has_value());
    value_ctor(value_);
    assert(value_.has_value());
  }

protected:
  ValueOpt value_;
};


}// namespace nut::aux