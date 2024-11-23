#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>

#include <net_utils/utils.hpp>


namespace nut {


/// Read-write spinlock with priority for writers
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
            {.readers = static_cast<StateCounter>(curr.readers + 1), .writers = 0, .w_lock = false},
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
    lock_exclusive(false);
  }


  bool try_lock() noexcept {
    return lock_exclusive(true);
  }


  void unlock() noexcept {
    auto curr = count_.load(std::memory_order_acquire);
    do {
      assert(curr.readers == 0);
      assert(curr.writers > 0);
      assert(curr.w_lock);
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
  bool lock_exclusive(bool const try_to_lock) noexcept {
    auto curr = count_.load(std::memory_order_acquire);
    do {
      if (try_to_lock && curr.w_lock) {
        return false;
      }

      if (State const desired{.readers = curr.readers, .writers = static_cast<StateCounter>(curr.writers + 1), .w_lock = curr.w_lock};
        count_.compare_exchange_strong(curr, desired, std::memory_order_acq_rel, std::memory_order_acquire)) {
        curr = desired;
        break;
      }

      thread_pause();
    } while (true);

    // wait readers and exclusive write
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

      thread_pause();
      curr = count_.load(std::memory_order_acquire);
      if (try_to_lock && curr.readers == 0 && curr.w_lock) {
        return false;
      }
    } while (true);

    return true;
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