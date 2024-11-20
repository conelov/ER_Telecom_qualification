#pragma once

#include <functional>
#include <memory>
#include <type_traits>


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


  template<typename Mod>
  std::enable_if_t<std::is_invocable_r_v<MutablePtr, Mod, MutablePtr>, void>
  modify(Mod&& mod) {
    do {
      MutablePtr p_old = std::atomic_load_explicit(&p_, std::memory_order_acquire);
      if (std::atomic_compare_exchange_strong_explicit(
            &p_,
            &p_old,
            mod(p_old ? std::make_shared<value_type>(*p_old) : std::make_shared<value_type>()),
            std::memory_order_release, std::memory_order_relaxed)) {
        break;
      }
    } while (true);
  }

private:
  MutablePtr p_;
};


}// namespace nut