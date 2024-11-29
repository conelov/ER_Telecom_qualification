#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_map>

#include <net_utils/utils.hpp>


#ifdef NUT_ARCH_X86
// https://stackoverflow.com/questions/58424276/why-can-mm-pause-significantly-improve-performance#comment103190748_58424276
  #define thread_pause() asm volatile("pause")

#else
  #error "thread_pause for this platform not implemented"

#endif


namespace nut {


template<bool strong, typename Atomic, typename Fn>
typename Atomic::value_type atomic_cas(std::shared_ptr<Atomic>& atomic, Fn&& fn, std::memory_order success = std::memory_order_acq_rel, std::memory_order failure = std::memory_order_acquire) noexcept {
  auto expected = atomic.load(std::memory_order_acquire);
  do {
    if constexpr (strong) {
      if (std::atomic_compare_exchange_strong_explicit(&atomic, expected, fn(expected), success, failure)) {
        return expected;
      }
    } else {
      if (std::atomic_compare_exchange_weak_explicit(&atomic, expected, fn(expected), success, failure)) {
        return expected;
      }
    }
    thread_pause();
  } while (true);
}


template<bool strong, typename Atomic, typename Fn>
typename Atomic::value_type atomic_cas(Atomic& atomic, Fn&& fn, std::memory_order success = std::memory_order_acq_rel, std::memory_order failure = std::memory_order_acquire) noexcept {
  auto expected = atomic.load(std::memory_order_acquire);
  do {
    if constexpr (strong) {
      if (atomic.compare_exchange_strong(expected, fn(expected), success, failure)) {
        return expected;
      }
    } else {
      if (atomic.compare_exchange_weak(expected, fn(expected), success, failure)) {
        return expected;
      }
    }
    thread_pause();
  } while (true);
}


template<typename Atomic, typename Fn>
typename Atomic::value_type atomic_cas(Atomic& atomic, Fn&& fn, bool strong = false, std::memory_order success = std::memory_order_acq_rel, std::memory_order failure = std::memory_order_acquire) noexcept {
  if (strong) {
    return atomic_cas<true>(atomic, std::forward<Fn>(fn), success, failure);
  } else {
    return atomic_cas<false>(atomic, std::forward<Fn>(fn), success, failure);
  }
}


class Latch final {
public:
  enum NotifyType : std::uint8_t {
    notify_one,
    notify_all,
  };

public:
  void wakeup(NotifyType notify) noexcept {
    switch (notify) {
      case notify_one:
        cv_.notify_one();
        break;
      case notify_all:
        cv_.notify_all();
        break;
      default:
        assert(false);
    }
  }


  template<typename Validator>
  void wait_unsafe(Validator&& validator) {
    if (!validator()) {
      wait(std::forward<Validator>(validator));
    }
  }


  template<typename Validator>
  void wait(Validator&& validator) {
    std::unique_lock lk{mx_};
    cv_.wait(lk, std::forward<Validator>(validator));
  }

private:
  std::condition_variable cv_;
  std::mutex              mx_;
};


template<typename Task_>
class TaskPool final {
public:
  using Task = Task_;
  // using Task = std::function<void()>;

private:
  using Counter = std::uint8_t;

  struct ThreadContext final {
    Task        task;
    std::thread thread;
  };

public:
  ~TaskPool() noexcept {
    next(true);
  }


  TaskPool(std::uint8_t thread_limit) noexcept
      : thread_limit_{thread_limit} {
  }


  void next(bool stop = false) noexcept {
    if (pool_.empty()) {
      assert(stop);
      return;
    }

    producer_latch_.wait_unsafe(WRAP_IN_LAMBDA_R(runner_initialized_.load(std::memory_order_seq_cst) == pool_.size(), this));
    if (stop) {
      iteration_.store(iteration_stop_, std::memory_order_seq_cst);
    } else {
      atomic_cas(iteration_, [](auto expected) {
        if (expected + 1 == iteration_stop_) {
          return 0;
        } else {
          return expected + 1;
        }
      });
    }
    assert(runner_busy_ == 0);
    runner_busy_.store(pool_.size(), std::memory_order_seq_cst);
    consumer_latch_.wakeup(Latch::notify_all);

    producer_latch_.wait_unsafe(WRAP_IN_LAMBDA_R(runner_busy_.load(std::memory_order_seq_cst) == 0, this));
    if (stop) {
      for (auto& [k, ctx] : pool_) {
        auto& thread = ctx.thread;
        if (thread.joinable()) {
          thread.join();
        }
      }
      pool_.clear();
    }
  }


  template<typename... TaskArgs>
  void bind(TaskArgs&&... task_args) {
    assert(pool_.find(std::this_thread::get_id()) == pool_.end());
    assert(pool_.size() < thread_limit_);

    std::thread thread{std::mem_fn(&TaskPool::runner), this};
    auto const  thread_id               = thread.get_id();
    [[maybe_unused]] auto const [it, f] = pool_.emplace(thread_id, ThreadContext{{std::forward<TaskArgs>(task_args)...}, std::move(thread)});
    assert(f);
  }

private:
  void runner() noexcept {
    runner_initialized_.fetch_add(1, std::memory_order_seq_cst);
    producer_latch_.wakeup(Latch::notify_one);
    auto const init_decrement = finally(WRAP_IN_LAMBDA(runner_initialized_.fetch_sub(1, std::memory_order_seq_cst) == 1 ? producer_latch_.wakeup(Latch::notify_one) : void(0), this));

    ThreadContext* ctx               = nullptr;
    auto           current_iteration = iteration_.load(std::memory_order_seq_cst);
    do {
      {
        Counter request_iteration;
        consumer_latch_.wait_unsafe(WRAP_IN_LAMBDA_R((request_iteration = iteration_.load(std::memory_order_seq_cst)) != current_iteration, this, current_iteration, &request_iteration));
        current_iteration = request_iteration;
      }
      assert(runner_busy_ > 0);
      auto const busy_decrement = finally(WRAP_IN_LAMBDA(runner_busy_.fetch_sub(1, std::memory_order_seq_cst) == 1 ? producer_latch_.wakeup(Latch::notify_one) : void(0), this));

      if (current_iteration == iteration_stop_) {
        break;
      }

      if (!ctx) {
        ctx = &pool_.find(std::this_thread::get_id())->second;
      }
      ctx->task();
    } while (true);
  }

private:
  static auto constexpr iteration_stop_ = std::numeric_limits<Counter>::max();

private:
  Latch                                              producer_latch_;
  Latch                                              consumer_latch_;
  std::unordered_map<std::thread::id, ThreadContext> pool_;
  std::atomic<Counter>                               iteration_          = 0;
  std::atomic<Counter>                               runner_initialized_ = 0;
  std::atomic<Counter>                               runner_busy_        = 0;
  std::uint8_t const                                 thread_limit_;
};


}// namespace nut