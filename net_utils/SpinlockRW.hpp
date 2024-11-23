#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>

#include <net_utils/utils.hpp>


namespace nut {


class SpinlockRW final {
public:
  ~SpinlockRW() {
#ifndef NDEBUG
    auto const [r, w, l] = count_.load(std::memory_order_seq_cst);
    assert(r == 0);
    assert(w == 0);
    assert(l == 0);
#endif
  }


  void lock_shared() noexcept {
    auto curr = count_.load(std::memory_order_acquire);
    do {
      // expect no writers
      curr.writers = 0;
      curr.w_lock  = false;
      if (count_.compare_exchange_strong(
            curr,
            State{.readers = static_cast<StateCounter>(curr.readers + 1), .writers = 0, .w_lock = false},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }


  void unlock_shared() noexcept {
    auto curr = count_.load(std::memory_order_acquire);
    do {
      assert(curr.readers > 0);
      assert(curr.w_lock == false);
      if (count_.compare_exchange_strong(
            curr,
            {.readers = static_cast<StateCounter>(curr.readers - 1), .writers = curr.writers, .w_lock = false},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }


  void lock() noexcept {
    auto  curr = count_.load(std::memory_order_acquire);
    State desired;
    do {
      if (count_.compare_exchange_strong(
            curr,
            desired = {.readers = curr.readers, .writers = static_cast<StateCounter>(curr.writers + 1), .w_lock = curr.w_lock},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);


    // wait readers and exclusive write
    curr = desired;
    do {
      curr.readers = 0;
      curr.w_lock  = false;
      if (count_.compare_exchange_weak(
            curr,
            {.readers = 0, .writers = curr.writers, .w_lock = true},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      curr = count_.load(std::memory_order_acquire);
    } while (true);
  }


  bool try_lock() noexcept {
    auto  curr = count_.load(std::memory_order_acquire);
    if (count_.compare_exchange_strong(
          curr,
          {.readers = 0, .writers = static_cast<StateCounter>(curr.writers + 1), .w_lock = true},
          std::memory_order_acq_rel,
          std::memory_order_acquire)) {
    }
  }


  void unlock() noexcept {
    auto curr = count_.load(std::memory_order_acquire);
    do {
      assert(curr.readers == 0);
      assert(curr.writers > 0);
      assert(curr.w_lock == true);
      if (count_.compare_exchange_strong(
            curr,
            {.readers = 0, .writers = static_cast<StateCounter>(curr.writers - 1), .w_lock = false},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }

private:
  using StateCounter = std::uint16_t;

  struct State final {
    StateCounter readers : sizeof(StateCounter) * 8;
    StateCounter writers : sizeof(StateCounter) * 8 - 1;
    bool         w_lock : 1;
  };

private:
  std::atomic<State> count_ = State{.readers = 0, .writers = 0};

  static_assert(decltype(count_)::is_always_lock_free);
};


}// namespace nut