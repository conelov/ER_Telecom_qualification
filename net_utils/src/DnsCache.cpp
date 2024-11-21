#include <cassert>
#include <cmath>
#include <queue>

#include <net_utils/DnsCache.hpp>


namespace nut {
namespace {
namespace _ {


constexpr auto kCacheSize = 9. / 10;


}// namespace _
}// namespace


DnsCache::~DnsCache() = default;


DnsCache::DnsCache(std::size_t cache_limit, std::size_t cache_size)
    : map_{std::make_shared<HashMap>()}
    , cache_limit_{cache_limit}
    , cache_size_{cache_size} {
  assert(cache_size > cache_limit);
}


DnsCache::DnsCache(std::size_t cache_limit)
    : DnsCache{cache_limit, cache_limit + static_cast<std::size_t>(std::ceil(std::pow(cache_limit, _::kCacheSize)))} {
}


DnsCache::DnsCache()
    : DnsCache{DNS_CACHE_REC_LIMIT} {
}


void DnsCache::update(const std::string& name, const std::string& ip_in) {
  auto const self = instance();
  self->map_.modify([self, &name, &ip_in](auto ptr) {
    assert(ptr);
    auto& [ip, time] = (*ptr)[name];
    ip               = ip_in;
    time             = Clock::now();
    self->cleanup_if_needed(*ptr);
    return std::move(ptr);
  });
}


std::string DnsCache::resolve(const std::string& name) {
  auto const self = instance();
  if (auto const lock = *self->map_) {
    if (auto const it = lock->find(name); it != lock->cend()) {
      return it->second.ip;
    }
  }
  return {};
}


// O(DNS_CACHE_REC_LIMIT log DNS_CACHE_CLEANUP_BOUND)
void DnsCache::cleanup_if_needed(HashMap& map) const {
  if (map.size() <= cache_size_) {
    return;
  }

  using It                  = HashMap::const_iterator;
  auto constexpr queue_comp = [](It lhs, It rhs) noexcept {
    return lhs->second.timestamp < rhs->second.timestamp;
  };
  std::vector<It> mem;
  mem.reserve(map.size() - cache_limit_ + 1);
  std::priority_queue<It, decltype(mem), decltype(queue_comp)> queue{queue_comp, std::move(mem)};

  for (auto it = map.cbegin(); it != map.cend(); ++it) {
    queue.push(it);
    if (queue.size() > cache_limit_) {
      queue.pop();
    }
  }

  while (!queue.empty()) {
    map.erase(queue.top());
    queue.pop();
  }
}


}// namespace nut
