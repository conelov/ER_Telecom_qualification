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

protected:
  void begin_iteration() override {
    value_ctor(value);
  }

protected:
  ValueOpt value;
};


}// namespace nut::aux