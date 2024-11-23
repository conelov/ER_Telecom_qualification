#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>

#include <net_utils/utils.hpp>


namespace nut {


template<typename RT_ = std::uint_least8_t, typename WT_ = std::uint_least8_t>
class SpinlockRW final {
  static_assert(std::is_integral_v<RT_> && std::is_integral_v<WT_>);

public:
  ~SpinlockRW() {
#ifndef NDEBUG
    auto const [r, w] = count_.load(std::memory_order_seq_cst);
    assert(r == 0);
    assert(w == 0);
#endif
  }


  void lock_shared() noexcept {
    auto curr = count_.load(std::memory_order_acquire);
    do {
      assert(curr.readers < std::numeric_limits<RT_>::max());
      // expect no writers
      curr.writers = 0;
      if (count_.compare_exchange_strong(
            curr,
            {.readers = curr.readers + RT_{1}, .writers = curr.writers},
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
      if (count_.compare_exchange_strong(
            curr,
            {.readers = curr.readers - RT_{1}, .writers = curr.writers},
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
    // fetch_add writers
    do {
      assert(curr.writers < std::numeric_limits<WT_>::max());
      desired = {.readers = curr.readers, .writers = curr.writers + WT_{1}};
      if (count_.compare_exchange_strong(
            curr,
            desired,
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);

    wait_exclusive(desired);
  }


  bool try_lock() noexcept {
    auto curr          = count_.load(std::memory_order_acquire);
    curr.writers       = 0;
    auto const desired = {.readers = curr.readers, .writers = 1};
    if (!count_.compare_exchange_strong(
          curr,
          desired,
          std::memory_order_acq_rel,
          std::memory_order_acquire)) {
      return false;
    }

    wait_exclusive(desired);
    return true;
  }


  void unlock() noexcept {
    auto curr = count_.load(std::memory_order_acquire);
    do {
      assert(curr.readers == 0);
      assert(curr.writers > 0);
      if (count_.compare_exchange_strong(
            curr,
            {.readers = curr.readers, .writers = curr.writers - WT_{1}},
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        break;
      }
      thread_pause();
    } while (true);
  }

private:
  struct State final {
    RT_ readers;
    WT_ writers;
  };

private:
  void wait_exclusive(State& curr) noexcept {
    if (curr.readers == 0 && curr.writers == 1) {
      return;
    }

    // wait other writers and readers
    do {
      if (auto const ex = count_.load(std::memory_order_acquire);
        ex.readers == 0 && ex.writers == 1) {
        break;
      }
      thread_pause();
    } while (true);
  }

private:
  std::atomic<State> count_ = State{.readers = 0, .writers = 0};

  static_assert(decltype(count_)::is_always_lock_free);
};


}// namespace nut