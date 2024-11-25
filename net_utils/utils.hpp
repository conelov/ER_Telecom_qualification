#pragma once

#include <tuple>
#include <utility>


#define INVOKER_AS_LAMBDA(fn, ...) [__VA_ARGS__](auto&&... args) constexpr noexcept(noexcept(fn(std::forward<decltype(args)>(args)...))) -> decltype(auto) { \
  return fn(std::forward<decltype(args)>(args)...);                                                                                                          \
}


#ifdef NUT_CXX_GNU_L_11
  #define MEM_FN_LAMBDA(mem, ...) [__VA_ARGS__](auto&& arg) constexpr -> decltype(auto) { \
    return std::forward<decltype(arg)>(arg) mem;                                          \
  }
#else
  #define MEM_FN_LAMBDA(mem, ...) [__VA_ARGS__](auto&& arg) noexcept(noexcept(std::forward<decltype(arg)>(arg) mem)) -> decltype(auto) { \
    return std::forward<decltype(arg)>(arg) mem;                                                                                         \
  }
#endif


#define WRAP_IN_LAMBDA(expr, ...) [__VA_ARGS__](auto&&...) constexpr noexcept(noexcept(expr)) -> decltype(auto) { \
  do {                                                                                                            \
    expr;                                                                                                         \
  } while (false);                                                                                                \
}


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
constexpr auto bind_front(Fn&& fn, Args&&... args) noexcept {
  return [fn = std::make_tuple(std::forward<Fn>(fn)), cap = std::make_tuple(std::forward<Args>(args)...)](auto&&... args) mutable {
    return std::apply(std::get<0>(fn), std::tuple_cat(std::forward<decltype(cap)>(cap), std::make_tuple(std::forward<decltype(args)>(args)...)));
  };
}


template<typename Fn, typename... Args>
constexpr auto bind_back(Fn&& fn, Args&&... args) noexcept {
  return [fn = std::make_tuple(std::forward<Fn>(fn)), cap = std::make_tuple(std::forward<Args>(args)...)](auto&&... args) mutable {
    return std::apply(std::get<0>(fn), std::tuple_cat(std::make_tuple(std::forward<decltype(args)>(args)...), std::forward<decltype(cap)>(cap)));
  };
}


}// namespace nut


#ifdef NUT_ARCH_X86
// https://stackoverflow.com/questions/58424276/why-can-mm-pause-significantly-improve-performance#comment103190748_58424276
  #define thread_pause() asm volatile("pause")

#else
  #error "thread_pause for this platform not implemented"

#endif
