#pragma once

#include <atomic>
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

private:
  class Deleter final {
  public:
    void operator()(value_type* ptr) const {
      if (!lock_->test_and_set(std::memory_order_relaxed)) {
        assert(ptr);
        std::default_delete<value_type>{}(ptr);
      }
    }

  private:
    std::shared_ptr<std::atomic_flag> mutable lock_ = [] {
      auto out = std::make_shared<std::atomic_flag>();// ATOMIC_FLAG_INIT not passed as a parameter
      out->clear();
      return out;
    }();
  };


private:
  template<typename... Args>
  MutablePtr make_ptr_value(Args&&... args) {
    return {new value_type(std::forward<Args>(args)...), Deleter{}};
  }


  MutablePtr make_nullptr_value() noexcept {
    return MutablePtr{nullptr, Deleter{}};
  }

public:
  template<typename... Args>
  RcuStorage(Args&&... args)
      : rcu_rev_{make_ptr_value(std::forward<Args>(args)...)} {
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
              p_new = mod(p_old ? make_ptr_value(*p_old) : make_nullptr_value()),
              std::memory_order_acq_rel, std::memory_order_acquire)) {
          break;
        }
        thread_pause();
      } while (true);
    }
    return p_new;
  }

private:
  MutablePtr rcu_rev_ = make_nullptr_value();
  Lock_      lock_;
};


}// namespace nut