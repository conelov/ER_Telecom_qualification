#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <type_traits>

#include <net_utils/utils.hpp>


namespace nut {


template<typename T_, typename Lock_>
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


  [[nodiscard]] ReadPtr load() const {
    return std::atomic_load_explicit(&p_, std::memory_order_acquire);
  }


  [[nodiscard]] ReadPtr operator*() {
    return load();
  }


  template<typename Mod>
  void modify(bool const exclusive, Mod&& mod) {
    if (exclusive) {
      modify_impl<true>(std::forward<Mod>(mod));
    } else {
      modify_impl<false>(std::forward<Mod>(mod));
    }
  }


  template<typename Mod>
  void modify(Mod&& mod) {
    modify_impl<false>(std::forward<Mod>(mod));
  }

private:
  template<bool exclusive, typename Mod>
  void modify_impl(Mod&& mod) {
    static_assert(std::is_invocable_r_v<MutablePtr, Mod, MutablePtr>);
    {
      auto const lk = [this]() noexcept {
        if constexpr (exclusive) {
          return std::unique_lock{lock_};
        } else {
          return std::shared_lock{lock_};
        }
      }();

      MutablePtr p_old = std::atomic_load_explicit(&p_, std::memory_order_acquire);
      do {
        if (std::atomic_compare_exchange_strong_explicit(
              &p_,
              &p_old,
              mod(p_old ? std::make_shared<value_type>(*p_old) : MutablePtr{}),
              std::memory_order_acq_rel, std::memory_order_acquire)) {
          break;
        }
        thread_pause();
      } while (true);
    }
  }

private:
  MutablePtr mutable p_;
  Lock_ lock_;
};


}// namespace nut