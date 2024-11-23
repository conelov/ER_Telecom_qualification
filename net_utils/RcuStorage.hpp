#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <type_traits>

#include <net_utils/SpinlockRW.hpp>
#include <net_utils/utils.hpp>


namespace nut {


template<typename T_>
class RcuStorage final {
public:
  using value_type = T_;
  using ReadPtr    = std::shared_ptr<value_type const>;
  using MutablePtr = std::shared_ptr<value_type>;

public:
  RcuStorage() noexcept = default;


  explicit RcuStorage(MutablePtr in)
      : p_{std::move(in)} {
  }


  [[nodiscard]] ReadPtr load() {
    return std::atomic_load_explicit(&p_, std::memory_order_relaxed);
  }


  [[nodiscard]] ReadPtr operator*() {
    return load();
  }


  template<typename Mod, bool try_exclusive = false>
  std::enable_if_t<std::is_invocable_r_v<MutablePtr, Mod, MutablePtr, bool>, void>
  modify(Mod&& mod) {
    auto const lk = [this]() noexcept {
      if constexpr (try_exclusive) {
        std::unique_lock lk{lock_, std::try_to_lock};
        return std::pair{std::move(lk), lk.owns_lock()};
      } else {
        return std::shared_lock{lock_};
      }
    }();

    do {
      MutablePtr p_old = std::atomic_load_explicit(&p_, std::memory_order_acquire);
      if (std::atomic_compare_exchange_strong_explicit(
            &p_,
            &p_old,
            mod(p_old ? std::make_shared<value_type>(*p_old) : MutablePtr{}),
            std::memory_order_release, std::memory_order_relaxed)) {
        break;
      }
      thread_pause();
    } while (true);
  }

private:
  using Lock = SpinlockRW<>;

private:
  MutablePtr p_;
  Lock       lock_;
};


}// namespace nut