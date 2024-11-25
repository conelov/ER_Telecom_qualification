#pragma once

#include <mutex>
#include <optional>
#include <shared_mutex>

#include <net_utils/aux/MultiThreadedRWValuedFixture.hpp>


namespace nut::aux {


using PriorityMutexFixtureData = std::vector<std::intmax_t>;


template<typename Mx_>
class PriorityMutexFixture : public MultiThreadedRWValuedFixture<PriorityMutexFixtureData> {
public:
  void bind() {
    value_ctor = [this](ValueOpt& opt) {
      if (!opt) {
        opt.emplace();
      }
      opt->assign(writers, 0);
    };

    MultiThreadedRWFixture::bind(
      [this](std::size_t idx, auto...) {
        {
          std::shared_lock const lk{mx_};
          auto volatile dummy = value->at(idx % writers);
          ++dummy;
          iter_counter_.fetch_add(1, std::memory_order_relaxed);
        }
      },
      [this](std::size_t idx, auto...) {
        {
          std::unique_lock const lk{mx_};
          ++value->at(idx);
          iter_counter_.fetch_add(1, std::memory_order_relaxed);
        }
      });
  }


  [[nodiscard]] std::size_t iter_counter() const {
    return iter_counter_.load(std::memory_order_acquire);
  }


protected:
  void pre_start() override {
    MultiThreadedRWValuedFixture::pre_start();
    iter_counter_.store(0, std::memory_order_release);
  }

private:
  Mx_                      mx_;
  std::atomic<std::size_t> iter_counter_ = 0;
};


}// namespace nut::aux