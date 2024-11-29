#pragma once

#include <mutex>
#include <optional>
#include <shared_mutex>

#include <net_utils/aux/MultiThreadedRWValuedFixture.hpp>


namespace nut::aux {


template<typename Mx_>
class PriorityMutexFixture : public MultiThreadedRWValuedFixture<std::vector<std::uintmax_t>> {
public:
  void bind() {
    using Base = MultiThreadedRWValuedFixture;
    value_ctor = [this](auto& opt) {
      if (!opt) {
        opt.emplace();
      }
      opt->clear();
      opt->assign(writers, 0);
    };

    Base::bind(
      [this](std::size_t idx, auto...) {
        {
          std::shared_lock const lk{mx_};
          auto volatile dummy = value->at(idx % writers);
          ++dummy;
        }
      },
      [this](std::size_t idx, auto...) {
        {
          std::unique_lock const lk{mx_};
          ++value->at(idx);
        }
      });
  }

private:
  Mx_ mx_;
};


}// namespace nut::aux