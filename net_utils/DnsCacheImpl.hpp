#pragma once

#include <chrono>
#include <mutex>
#include <type_traits>

#include <net_utils/LruStorage.hpp>
#include <net_utils/PriorityMutex.hpp>
#include <net_utils/RcuStorage.hpp>


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
  [[nodiscard]] std::string resolve(const std::string& name);

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
  Storage           st_;
  std::size_t const cache_size_;
  std::size_t const cache_cap_;
};

extern template class DnsCacheImplRcu<std::mutex>;
extern template class DnsCacheImplRcu<PriorityMutex>;


template<typename Mx_>
class DnsCacheImplLRU {
public:
  ~DnsCacheImplLRU();
  DnsCacheImplLRU(std::size_t cache_size);

  void                      update(const std::string& name, const std::string& ip);
  [[nodiscard]] std::string resolve(const std::string& name);

private:
  LruStorage<std::string, std::string, std::string_view> mutable st_;
  Mx_ mx_;
};


extern template class DnsCacheImplLRU<std::mutex>;
extern template class DnsCacheImplLRU<PriorityMutex>;


}// namespace aux


enum class DnsCacheImplType {
  rcu_std_mx,
  rcu_priority_mutex,
  lru_std_mx,
  lru_priority_mutex,
};


template<DnsCacheImplType type>
using DnsCacheImpl =
  std::conditional_t<type == DnsCacheImplType::rcu_std_mx,
    aux::DnsCacheImplRcu<std::mutex>,
    //
    std::conditional_t<type == DnsCacheImplType::rcu_priority_mutex,
      aux::DnsCacheImplRcu<PriorityMutex>,
      //
      std::conditional_t<type == DnsCacheImplType::lru_std_mx,
        aux::DnsCacheImplLRU<std::mutex>,
        //
        /// type == DnsCacheImplType::lru_priority_mutex
        aux::DnsCacheImplLRU<PriorityMutex>
        //
        >>>;


}// namespace nut
