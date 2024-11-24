#pragma once

#include <tuple>
#include <utility>


namespace nut {


template<typename F>
[[nodiscard]] constexpr auto finally(F&& f) noexcept {
  class Sentry final {
  public:
    ~Sentry() noexcept {
      if (!spent_) {
        std::move(f_)();
      }
    }

    constexpr explicit Sentry(F&& in) noexcept
        : f_{std::move(in)} {
    }

    constexpr Sentry(Sentry&& in) noexcept
        : f_{std::move(in.f_)} {
      in.spent_ = true;
    }

    constexpr Sentry(Sentry const&) noexcept = delete;

  private:
    F    f_;
    bool spent_ = false;
  };

  return Sentry{std::forward<F>(f)};
}


template<typename Fn, typename... Args>
constexpr auto carry(Fn&& fn, Args&&... args) noexcept {
  return [fn = std::make_tuple(std::forward<Fn>(fn)), cap = std::make_tuple(std::forward<Args>(args)...)](auto&&... args) mutable {
    return std::apply(std::get<0>(fn), std::tuple_cat(cap, std::make_tuple(std::forward<decltype(args)>(args)...)));
  };
}


}// namespace nut
