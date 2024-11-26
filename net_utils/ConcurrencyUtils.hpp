#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>

#include <net_utils/utils.hpp>


#ifdef NUT_ARCH_X86
// https://stackoverflow.com/questions/58424276/why-can-mm-pause-significantly-improve-performance#comment103190748_58424276
  #define thread_pause() asm volatile("pause")

#else
  #error "thread_pause for this platform not implemented"

#endif


namespace nut {


class Latch final {
public:
  template<typename Validator>
  void wait(Validator&& validator) {
    {
      std::unique_lock lk{mx_};
      cv_.wait(lk, std::forward<Validator>(validator));
    }
    wakeup();
  }


  void wakeup() noexcept {
    cv_.notify_one();
  }

private:
  std::condition_variable cv_;
  std::mutex              mx_;
};


class CrossBarrier final {
public:
  CrossBarrier() noexcept = default;


  CrossBarrier(std::size_t consumers) noexcept {
    reset(consumers);
  }


  void reset(std::size_t consumers) noexcept {
    assert(state_ == State::parked);
    consumers_ = consumers;
  }


  [[nodiscard]] auto consumer_wait() noexcept {
    auto const validator = WRAP_IN_LAMBDA_R(state_.load(std::memory_order_relaxed) == State::running, this);
    if (!validator()) {
      consumer_latch_.wait(validator);
      assert(sema_consumers_ != 0);
    }
    return finally(WRAP_IN_LAMBDA_R(consumer_finished(), this));
  }


  void consumers_run() noexcept {
    assert(state_ == State::parked);
    assert(sema_consumers_ == 0);
    sema_consumers_.store(consumers_, std::memory_order_relaxed);
    state_.store(State::running, std::memory_order_relaxed);
    consumer_latch_.wakeup();
  }


  void producer_wait() noexcept {
    auto const validator = WRAP_IN_LAMBDA_R(sema_consumers_.load(std::memory_order_relaxed) == 0, this);
    if (validator()) {
      return;
    }
    producer_latch_.wait(validator);
  }

private:
  enum class State : std::uint8_t {
    parked,
    running,
  };

private:
  void consumer_finished() noexcept {
    assert(state_ == State::running);
    assert(sema_consumers_ > 0);
    if (sema_consumers_.fetch_sub(1, std::memory_order_relaxed) - 1 == 0) {
      state_.store(State::parked, std::memory_order_relaxed);
      producer_latch_.wakeup();
    }
  }

private:
  Latch producer_latch_;
  Latch consumer_latch_;

  std::atomic<std::uint16_t> sema_consumers_ = 0;
  std::uint16_t              consumers_      = 0;

  std::atomic<State> state_ = State::parked;
};


}// namespace nut