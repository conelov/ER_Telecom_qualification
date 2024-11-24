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
  ~SpinlockRW() {
#ifndef NDEBUG
    auto const [r, w, l] = v_.load();
    assert(r == 0);
    assert(w == 0);
    assert(!l);
#endif
  }


  void lock_shared() noexcept {
    // expect no writer
    do {
      auto exp = v_.load(std::memory_order_acquire);
      assert(exp.readers < std::numeric_limits<StateCounter>::max());
      exp.writers = 0;
      exp.w_lock  = false;
      if (v_.compare_exchange_weak(
            exp,
            {static_cast<StateCounter>(exp.readers + 1), 0, false},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }


  void unlock_shared() noexcept {
    do {
      auto exp = v_.load(std::memory_order_acquire);
      assert(exp.readers > 0);
      assert(!exp.w_lock);
      if (v_.compare_exchange_weak(
            exp,
            {static_cast<StateCounter>(exp.readers - 1), exp.writers, false},
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
      assert(exp.writers < std::numeric_limits<StateCounter>::max() / 2);
      if (State const desired = {exp.readers, static_cast<StateCounter>(exp.writers + 1), exp.w_lock};
        v_.compare_exchange_weak(
          exp,
          desired,
          std::memory_order_acq_rel,
          std::memory_order_acquire)) {
        exp = desired;
        break;
      }
      thread_pause();
    } while (true);

    // wait readers and exclusive write
    do {
      exp.readers = 0;
      exp.w_lock  = false;
      if (v_.compare_exchange_weak(
            exp,
            {0, exp.writers, true},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }


  void unlock() noexcept {
    auto curr = v_.load(std::memory_order_acquire);
    do {
      assert(curr.readers == 0);
      assert(curr.writers > 0);
      assert(curr.w_lock == true);
      if (v_.compare_exchange_strong(
            curr,
            {0, static_cast<StateCounter>(curr.writers - 1), false},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }

private:
  using StateCounter = Count_;

  struct State final {
    StateCounter readers : sizeof(StateCounter) * 8;
    StateCounter writers : sizeof(StateCounter) * 8 - 1;
    bool         w_lock : 1;
  };

private:
  std::atomic<State> v_ = State{.readers = 0, .writers = 0};

  static_assert(decltype(v_)::is_always_lock_free);
};


}// namespace nut