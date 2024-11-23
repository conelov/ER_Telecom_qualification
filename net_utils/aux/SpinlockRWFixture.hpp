#pragma once

#include <mutex>
#include <optional>
#include <shared_mutex>

#include <net_utils/SpinlockRW.hpp>

#include <net_utils/aux/MultiThreadedRWFixture.hpp>


namespace nut::aux {


template<typename Mx_, bool try_lock_mode>
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
        auto const do_lock = [this] {
          if constexpr (try_lock_mode) {
            do {
              auto lk = std::unique_lock{*sl_, std::try_to_lock};
              if (lk) {
                return lk;
              }
            } while (true);
          } else {
            return std::unique_lock{*sl_};
          }
        };
        {
          auto const     lk    = do_lock();
          auto volatile& dummy = data;
          ++dummy;
        }
        {
8          auto const     lk    = do_lock();
          auto volatile& dummy = data;
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