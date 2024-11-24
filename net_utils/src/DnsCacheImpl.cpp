#include <cassert>
#include <cmath>
#include <mutex>
#include <queue>

#include <net_utils/DnsCacheImpl.hpp>
#include <net_utils/SpinlockRW.hpp>


namespace nut {
namespace aux {
namespace {


constexpr auto kCacheCap = 9. / 10;


}// namespace


template<typename Mx_>
DnsCacheImplRcu<Mx_>::~DnsCacheImplRcu() = default;


template<typename Mx_>
DnsCacheImplRcu<Mx_>::DnsCacheImplRcu(std::size_t cache_size, std::size_t cache_cap)
    : map_{std::make_shared<HashMap>()}
    , cache_size_{cache_size}
    , cache_cap_{cache_cap} {
  assert(cache_cap_ >= cache_size_);
}


template<typename Mx_>
DnsCacheImplRcu<Mx_>::DnsCacheImplRcu(std::size_t cache_size)
    : DnsCacheImplRcu{
        cache_size,
        cache_size + static_cast<std::size_t>(std::ceil(std::pow(cache_size, kCacheCap)))// heuristic predict cap
      } {
}


template<typename Mx_>
DnsCacheImplRcu<Mx_>::DnsCacheImplRcu()
    : DnsCacheImplRcu{DNS_CACHE_REC_LIMIT} {
}


template<typename Mx_>
void DnsCacheImplRcu<Mx_>::update(const std::string& name, const std::string& ip_in) const {
  map_.modify([this, &name, &ip_in](auto ptr) {
    assert(ptr);
    auto& [ip, time] = (*ptr)[name];
    ip               = ip_in;
    time             = Clock::now();
    cleanup_if_needed(*ptr);
    return std::move(ptr);
  });
}


template<typename Mx_>
std::string DnsCacheImplRcu<Mx_>::resolve(const std::string& name) {
  if (auto const lock = *map_) {
    if (auto const it = lock->find(name); it != lock->cend()) {
      return it->second.ip;
    }
  }
  return {};
}


template<typename Mx_>
void DnsCacheImplRcu<Mx_>::cleanup_if_needed(HashMap& map) const {
  if (map.size() <= cache_cap_) {
    return;
  }

  // O(size * log(size - cap))
  using It                  = typename HashMap::const_iterator;
  auto constexpr queue_comp = [](It lhs, It rhs) noexcept {
    return lhs->second.timestamp < rhs->second.timestamp;
  };
  std::vector<It> mem;
  auto const      surplus = map.size() - cache_size_;
  mem.reserve(surplus + 1);
  std::priority_queue<It, decltype(mem), decltype(queue_comp)> queue{queue_comp, std::move(mem)};

  for (auto it = map.cbegin(); it != map.cend(); ++it) {
    queue.push(it);
    if (queue.size() > surplus) {
      queue.pop();
    }
  }

  while (!queue.empty()) {
    map.erase(queue.top());
    queue.pop();
  }
}

template<>
class DnsCacheImplRcu<std::mutex>;

template<>
class DnsCacheImplRcu<SpinlockRW<>>;


template<typename Mx_>
DnsCacheImplLRU<Mx_>::~DnsCacheImplLRU() = default;


template<typename Mx_>
DnsCacheImplLRU<Mx_>::DnsCacheImplLRU() = default;


template<typename Mx_>
void DnsCacheImplLRU<Mx_>::update(const std::string& name, const std::string& ip) const {

}


template<typename Mx_>
std::string DnsCacheImplLRU<Mx_>::resolve(const std::string& name) {
  auto const it = map_.find(name);
  if (it == map_.end()) {
    return {};
  }

  list_.splice(list_.cbegin(), list_, it->second);
  return it->second->second;
}


}// namespace aux


}// namespace nut
