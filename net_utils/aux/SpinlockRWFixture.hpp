#pragma once

#include <mutex>
#include <optional>
#include <shared_mutex>

#include <net_utils/SpinlockRW.hpp>

#include <net_utils/aux/MultiThreadedRWFixture.hpp>


namespace nut::aux {


template<typename Mx_>
class SpinlockRWFixture : public MultiThreadedRWFixture {
public:
  int data;

public:
  void up(std::size_t r_iters, std::size_t w_iters) {
    sl_.emplace();
    data = 0;
    iter_counter_.store(0, std::memory_order_release);

    MultiThreadedRWFixture::up(
      r_iters,
      w_iters,
      [this](auto...) {
        {
          std::shared_lock const lk{*sl_};
          [[maybe_unused]] auto volatile dummy = data;
          ++dummy;
        }
        iter_counter_.fetch_add(1, std::memory_order_relaxed);
      },
      [this](auto...) {
        {
          std::unique_lock const lk{*sl_};
          auto volatile&         dummy = data;
          ++dummy;
        }
        {
          std::unique_lock const lk{*sl_};
          auto volatile&         dummy = data;
          --dummy;
        }
        iter_counter_.fetch_add(1, std::memory_order_relaxed);
        thread_pause();
      });
  }


  void down() override {
    MultiThreadedRWFixture::down();
    sl_.reset();
  }


  [[nodiscard]] std::size_t iter_counter() const {
    return iter_counter_.load(std::memory_order_acquire);
  }

private:
  std::optional<Mx_>       sl_;
  std::atomic<std::size_t> iter_counter_;
};


}// namespace nut::aux