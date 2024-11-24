#pragma once

#include <chrono>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include <net_utils/LruStorage.hpp>
#include <net_utils/RcuStorage.hpp>
#include <net_utils/SpinlockRW.hpp>


namespace nut {
namespace aux {


template<typename Mx_>
class DnsCacheImplRcu {
public:
  ~DnsCacheImplRcu();

  DnsCacheImplRcu(std::size_t cache_size, std::size_t cache_cap);
  DnsCacheImplRcu(std::size_t cache_size);
  DnsCacheImplRcu();

  void                      update(const std::string& name, const std::string& ip);
  [[nodiscard]] std::string resolve(const std::string& name) const;

private:
  using Clock = std::chrono::steady_clock;

  struct Record final {
    std::string       ip;
    Clock::time_point timestamp;
  };

  using HashMap = std::unordered_map<std::string, Record>;
  using Storage = RcuStorage<HashMap, Mx_>;

private:
  void cleanup_if_needed(HashMap& map) const;

private:
  Storage mutable st_;
  std::size_t const cache_size_;
  std::size_t const cache_cap_;
};

extern template class DnsCacheImplRcu<std::shared_mutex>;
// extern template class DnsCacheImplRcu<SpinlockRW<>>;


template<typename Mx_>
class DnsCacheImplLRU {
public:
  ~DnsCacheImplLRU();
  DnsCacheImplLRU(std::size_t cache_size);

  void                      update(const std::string& name, const std::string& ip);
  [[nodiscard]] std::string resolve(const std::string& name) const;

private:
  LruStorage<std::string, std::string, std::string_view> mutable st_;
  Mx_ mutable mx_;
};


extern template class DnsCacheImplLRU<std::shared_mutex>;
// extern template class DnsCacheImplLRU<SpinlockRW<>>;


}// namespace aux


enum class DnsCacheImplType {
  rcu_std_mx,
  rcu_spinlock_rw,
  lru_std_mx,
  lru_spinlock_rw,
};


template<DnsCacheImplType type>
using DnsCacheImpl =
  std::conditional_t<type == DnsCacheImplType::rcu_std_mx,
    aux::DnsCacheImplRcu<std::shared_mutex>,
    //
    // std::conditional_t<type == DnsCacheImplType::rcu_spinlock_rw,
    // aux::DnsCacheImplRcu<SpinlockRW<>>,
    //
    // std::conditional_t<type == DnsCacheImplType::lru_std_mx,
    aux::DnsCacheImplLRU<std::shared_mutex>
    // type == DnsCacheImplType::lru_spinlock_rw
    // aux::DnsCacheImplLRU<SpinlockRW<>>
    //
    // >
    // >
    >;


}// namespace nut
