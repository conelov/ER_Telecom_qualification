#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <type_traits>

#include <net_utils/utils.hpp>


namespace nut {


template<typename T_, typename Lock_, bool rw_mutex_ = false>
class RcuStorage {
public:
  using value_type = T_;
  using ReadPtr    = std::shared_ptr<value_type const>;
  using MutablePtr = std::shared_ptr<value_type>;

public:
  RcuStorage() noexcept = default;


  explicit RcuStorage(MutablePtr in)
      : rcu_rev_{std::move(in)} {
  }


  [[nodiscard]] ReadPtr load() const {
    return std::atomic_load_explicit(&rcu_rev_, std::memory_order_relaxed);
  }


  [[nodiscard]] ReadPtr operator*() {
    return load();
  }


  template<typename Mod, bool rw_mutex = rw_mutex_>
  std::enable_if_t<rw_mutex, ReadPtr> modify(bool const exclusive, Mod&& mod) {
    if (exclusive) {
      return modify_impl<true>(std::forward<Mod>(mod));
    } else {
      return modify_impl<false>(std::forward<Mod>(mod));
    }
  }


  template<typename Mod>
  ReadPtr modify(Mod&& mod) {
    return modify_impl<false>(std::forward<Mod>(mod));
  }

private:
  template<bool exclusive, typename Mod>
  ReadPtr modify_impl(Mod&& mod) {
    static_assert(std::is_invocable_r_v<MutablePtr, Mod, MutablePtr>);
    MutablePtr p_new;
    {
      auto const lk = [this]() noexcept {
        if constexpr (rw_mutex_ && exclusive) {
          return std::shared_lock{lock_};
        } else {
          return std::unique_lock{lock_};
        }
      }();

      MutablePtr p_old = std::atomic_load_explicit(&rcu_rev_, std::memory_order_acquire);
      do {
        if (std::atomic_compare_exchange_strong_explicit(
              &rcu_rev_,
              &p_old,
              p_new = mod(p_old ? std::make_shared<value_type>(*p_old) : MutablePtr{}),
              std::memory_order_acq_rel, std::memory_order_acquire)) {
          break;
        }
        thread_pause();
      } while (true);
    }
    return p_new;
  }

private:
  MutablePtr rcu_rev_;
  Lock_      lock_;
};


}// namespace nut