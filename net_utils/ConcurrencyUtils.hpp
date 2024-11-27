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


class ProduceConsume {
public:
  class InterfaceBase {
  public:
    InterfaceBase(InterfaceBase const&) = delete;
    InterfaceBase(InterfaceBase&&)      = delete;

  protected:
    InterfaceBase(ProduceConsume& self) noexcept
        : self_{self} {
    }

  protected:
    ProduceConsume& self_;
  };


  class ProducerInterface final : public InterfaceBase {
  public:
    void run(std::size_t consumers) noexcept {
      assert(consumers > 0);
      wait();
      self_.sema_consumers_.store(self_.consumers_ = consumers, std::memory_order_acq_rel);
      self_.iteration_.fetch_add(1, std::memory_order_seq_cst);
      self_.state_.store(State::running, std::memory_order_seq_cst);
      self_.consumer_latch_.wakeup();
    }


    void quit() noexcept {
      self_.state_.store(State::quit, std::memory_order_seq_cst);
      self_.consumer_latch_.wakeup();
      wait();
    }


    [[nodiscard]] bool is_running() const noexcept {
      return self_.sema_consumers_.load(std::memory_order_seq_cst) != 0;
    }


    void wait() noexcept {
      self_.producer_latch_.wait(WRAP_IN_LAMBDA_R(!is_running(), this));
    }

  private:
    using InterfaceBase::InterfaceBase;

    friend ProduceConsume;
  };


  class ConsumerInterface final : public InterfaceBase {
  public:
    template<typename Fn>
    void execute(Fn&& fn) noexcept {
      do {
        bool stopped_f = false;
        self_.consumer_latch_.wait(WRAP_IN_LAMBDA_R(
          self_.iteration_.load(std::memory_order_seq_cst) != 0 || (stopped_f = is_stopped()), this, &stopped_f));
        if (stopped_f) {
          break;
        }
        fn();
      } while (true);

      if (self_.sema_consumers_.fetch_sub(1, std::memory_order_seq_cst) - 1 == 0) {
        self_.producer_latch_.wakeup();
      }
    }


  private:
    using InterfaceBase::InterfaceBase;

    [[nodiscard]] bool is_stopped() const noexcept {
      return self_.state_.load(std::memory_order_seq_cst) == State::quit;
    }

    friend ProduceConsume;
  };

public:
  ProducerInterface producer = *this;
  ConsumerInterface consumer = *this;

public:
  ~ProduceConsume() noexcept {
    producer.quit();
  }

private:
  enum class State : std::uint8_t {
    running,
    quit,
  };

private:
  Latch producer_latch_;
  Latch consumer_latch_;

  std::atomic<std::uintmax_t> iteration_      = 0;
  std::atomic<std::uint16_t>  sema_consumers_ = 0;
  std::uint16_t               consumers_      = 0;
  std::atomic<State>          state_          = State::running;
};


}// namespace nut