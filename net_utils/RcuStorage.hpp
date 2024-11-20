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


  template<typename T>
  explicit RcuStorage(T&& in)
      : p_{in} {
  }


  [[nodiscard]] ReadPtr load() noexcept {
    return std::atomic_load_explicit(&p_, std::memory_order_relaxed);
  }


  [[nodiscard]] ReadPtr operator*() const noexcept {
    return load();
  }


  template<typename Mod, typename Merge>
  std::enable_if_t<std::is_invocable_v<Merge, value_type&, value_type const&>, void> modify(Mod&& mod, Merge&& merge) {
    MutablePtr const new_p = std::make_shared<value_type>(*load());
    std::invoke(std::forward<Mod>(mod), new_p);
    MutablePtr curr_p;
    do {
      curr_p = std::atomic_load_explicit(&p_, std::memory_order_acquire);
      merge(*new_p, *curr_p);
    } while (!std::atomic_compare_exchange_strong(&p_, &curr_p, new_p));
  }

private:
  MutablePtr p_;
};


}// namespace nut