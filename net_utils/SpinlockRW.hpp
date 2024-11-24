#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>

#include <net_utils/utils.hpp>


namespace nut {


/// Read-write spinlock with priority for writers
template<typename Count_ = std::uint16_t>
class SpinlockRW final {
public:
#ifndef NDEBUG
  ~SpinlockRW() {
    auto const [r, l] = v_.load();
    assert(r == 0);
    assert(!l);
  }
#endif


  void lock_shared() noexcept {
    auto exp = v_.load(std::memory_order_acquire);
    do {
      assert(exp.readers < std::numeric_limits<Counter>::max() / 2);
      exp.w_lock = false;
      if (v_.compare_exchange_weak(
            exp,
            {static_cast<Counter>(exp.readers + 1), false},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }


  void unlock_shared() noexcept {
    auto exp = v_.load(std::memory_order_acquire);
    do {
      assert(exp.readers > 0);
      if (v_.compare_exchange_weak(
            exp,
            {static_cast<Counter>(exp.readers - 1), exp.w_lock},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }


  void lock() noexcept {
    auto exp = v_.load(std::memory_order_acquire);
    do {
      exp.w_lock = false;
      if (State const desired = {exp.readers, true}; v_.compare_exchange_weak(
            exp,
            desired,
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        exp = desired;
        break;
      }
      thread_pause();
    } while (true);

    if (exp.readers == 0) {
      return;
    }

    do {
      exp.readers = 0;
      assert(exp.w_lock == true);
      if (v_.compare_exchange_weak(
            exp,
            {0, true},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }


  void unlock() noexcept {
    auto exp = v_.load(std::memory_order_acquire);
    do {
      assert(exp.readers == 0);
      assert(exp.w_lock);
      if (v_.compare_exchange_weak(
            exp,
            {exp.readers, false},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }

private:
  using Counter = Count_;

  struct alignas(sizeof(Counter)) State final {
    Counter readers : sizeof(Counter) * 8 - 1;
    bool    w_lock : 1;
  };

  static_assert(sizeof(State) == sizeof(Counter));
  static_assert(alignof(State) == alignof(Counter));

private:
  std::atomic<State> v_ = State{.readers = 0, .w_lock = false};

  static_assert(decltype(v_)::is_always_lock_free);
};


}// namespace nut