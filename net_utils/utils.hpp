#pragma once

#include <tuple>
#include <type_traits>
#include <utility>


namespace nut {


template<class T>
struct remove_cvref {
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};


template<class T>
using remove_cvref_t = typename remove_cvref<T>::type;


}// namespace nut


#define INVOKER_AS_LAMBDA(fn, ...) [__VA_ARGS__](auto&&... args) constexpr noexcept(noexcept(fn(std::forward<decltype(args)>(args)...))) -> decltype(auto) { \
  return fn(std::forward<decltype(args)>(args)...);                                                                                                          \
}

#ifdef NET_UTILS_CXX_GNU_L_11


  #define MEM_FN_LAMBDA(mem, ...) [__VA_ARGS__](auto&& arg) -> decltype(auto) { \
    if constexpr (std::is_pointer_v<decltype(arg)>) {                           \
      return std::forward<decltype(arg)>(arg)->mem;                             \
    } else {                                                                    \
      return std::forward<decltype(arg)>(arg).mem;                              \
    }                                                                           \
  }

  #define WRAP_IN_LAMBDA(expr, ...) [__VA_ARGS__](auto&&...) constexpr -> void { \
    do {                                                                         \
      expr;                                                                      \
    } while (false);                                                             \
  }


  #define WRAP_IN_LAMBDA_R(expr, ...) [__VA_ARGS__](auto&&...) constexpr -> decltype(auto) { \
    do {                                                                                     \
      return expr;                                                                           \
    } while (false);                                                                         \
  }

#else


  #define MEM_FN_LAMBDA(mem, ...) [__VA_ARGS__](auto&& arg) noexcept(noexcept(std::declval<nut::remove_cvref_t<std::remove_pointer_t<decltype(arg)>>>().mem)) -> decltype(auto) { \
    if constexpr (std::is_pointer_v<decltype(arg)>) {                                                                                                                             \
      return std::forward<decltype(arg)>(arg)->mem;                                                                                                                               \
    } else {                                                                                                                                                                      \
      return std::forward<decltype(arg)>(arg).mem;                                                                                                                                \
    }                                                                                                                                                                             \
  }


  #define WRAP_IN_LAMBDA(expr, ...) [__VA_ARGS__](auto&&...) constexpr noexcept(noexcept(expr)) -> void { \
    do {                                                                                                  \
      expr;                                                                                               \
    } while (false);                                                                                      \
  }


  #define WRAP_IN_LAMBDA_R(expr, ...) [__VA_ARGS__](auto&&...) constexpr noexcept(noexcept(expr)) -> decltype(auto) { \
    do {                                                                                                              \
      return expr;                                                                                                    \
    } while (false);                                                                                                  \
  }

#endif


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
      in.discard();
    }

    constexpr Sentry(Sentry const&) noexcept = delete;

    void discard() noexcept {
      assert(!spent_);
      spent_ = true;
    }

  private:
    F    f_;
    bool spent_ = false;
  };

  return Sentry{std::forward<F>(f)};
}


template<typename Fn, typename... Args>
constexpr decltype(auto) bind_front(Fn&& fn, Args&&... args) noexcept {
  return [fn = std::make_tuple(std::forward<Fn>(fn)), cap = std::make_tuple(std::forward<Args>(args)...)](auto&&... args) mutable {
    return std::apply(std::get<0>(fn), std::tuple_cat(cap, std::make_tuple(std::forward<decltype(args)>(args)...)));
  };
}


template<typename Fn, typename... Args>
constexpr decltype(auto) bind_front_once(Fn&& fn, Args&&... args) noexcept {
  return [fn = std::make_tuple(std::forward<Fn>(fn)), cap = std::make_tuple(std::forward<Args>(args)...), called = false](auto&&... args) mutable {
    if (called) {
      throw std::runtime_error{"Binded function is consumed."};
    }
    called = false;
    return std::apply(std::get<0>(std::move(fn)), std::tuple_cat(std::move(cap), std::make_tuple(std::forward<decltype(args)>(args)...)));
  };
}


template<typename Fn, typename... Args>
constexpr decltype(auto) bind_back(Fn&& fn, Args&&... args) noexcept {
  return [fn = std::make_tuple(std::forward<Fn>(fn)), cap = std::make_tuple(std::forward<Args>(args)...)](auto&&... args) mutable {
    return std::apply(std::get<0>(fn), std::tuple_cat(std::make_tuple(std::forward<decltype(args)>(args)...), cap));
  };
}


template<typename Fn, typename... Args>
constexpr decltype(auto) bind_back_once(Fn&& fn, Args&&... args) noexcept {
  return [fn = std::make_tuple(std::forward<Fn>(fn)), cap = std::make_tuple(std::forward<Args>(args)...), called = false](auto&&... args) mutable {
    if (called) {
      throw std::runtime_error{"Binded function is consumed."};
    }
    called = false;
    return std::apply(std::get<0>(std::move(fn)), std::tuple_cat(std::make_tuple(std::forward<decltype(args)>(args)...), std::move(cap)));
  };
}


template<typename>
struct is_pair : std::false_type {};

template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type {};


}// namespace nut
