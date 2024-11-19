#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <net_utils/utils.hpp>


namespace nut {


enum class SingletonLivetimeMode : std::uint8_t {
  Global,
  ThreadLocal
};


namespace aux {


template<typename Derived_, SingletonLivetimeMode>
struct SingletonMem;


template<typename Derived_>
struct SingletonMem<Derived_, SingletonLivetimeMode::Global> {
  static inline Derived_* p_;

  // https://en.cppreference.com/w/cpp/memory/destroy_at
  // https://habr.com/ru/post/540954/
  static std::byte* memory() {
    alignas(Derived_) static std::byte mem_[sizeof(Derived_)];
    return mem_;
  }
};


template<typename Derived_>
struct SingletonMem<Derived_, SingletonLivetimeMode::ThreadLocal> {
  static thread_local inline Derived_* p_;

  static std::byte* memory() {
    alignas(Derived_) static thread_local std::byte mem_[sizeof(Derived_)];
    return mem_;
  }
};


}// namespace aux


template<typename Derived_, SingletonLivetimeMode LMode_ = SingletonLivetimeMode::ThreadLocal, bool auto_init_ = LMode_ == SingletonLivetimeMode::ThreadLocal>
class Singleton : private aux::SingletonMem<Derived_, LMode_> {
  static_assert((LMode_ == SingletonLivetimeMode::Global && !auto_init_)
    || LMode_ == SingletonLivetimeMode::ThreadLocal);

private:
  using Derived = Derived_;
  using Mem     = aux::SingletonMem<Derived_, LMode_>;

public:
  template<typename... Args>
  static Derived* init(Args&&... args) {
    assert(!Mem::p_ && "Singleton has been initialized");
    Mem::p_ = new (Mem::memory()) Derived{std::forward<Args>(args)...};
    return instance();
  }


  [[nodiscard]] static Derived* instance() {
    if constexpr (auto_init_) {
      if (!Mem::p_) {
        init();
      }
    } else {
      assert(Mem::p_ && "Singleton not initialized");
    }
    return Mem::p_;
  }


  static void destroy() {
    if (Mem::p_) {
      Mem::p_->~Derived();
      Mem::p_ = nullptr;
    }
  }

protected:
  Singleton() noexcept = default;

private:
  [[maybe_unused]] static inline auto const destroy_auto_ = finally([] { destroy(); });
};


}// namespace nut