#include <cassert>
#include <queue>

#include <net_utils/DnsCache.hpp>


namespace nut {
namespace {
namespace _ {


}// namespace _
}// namespace


DnsCache::~DnsCache() = default;


DnsCache::DnsCache(std::size_t cache_limit, std::size_t cache_size)
    : map_{[cache_size] {
      auto out = std::make_shared<HashMap>();
      out->reserve(cache_size);
      return out;
    }()}
    , cache_limit_{cache_limit}
    , cache_size_{cache_size} {
  assert(cache_size > cache_size);
}


DnsCache::DnsCache(std::size_t cache_limit)
    : DnsCache{cache_limit, cache_limit + static_cast<std::size_t>(std::ceil(0.98 * std::sqrt(DNS_CACHE_REC_LIMIT)))} {
}


void DnsCache::update_global(const std::string& name, const std::string& ip) {
  instance()->update(name, ip);
}


std::string DnsCache::resolve_global(const std::string& name) {
  return instance()->resolve(name);
}


void DnsCache::update(const std::string& name, const std::string& ip_in) {
  map_.modify([&](auto ptr) {
    assert(ptr);
    auto& [ip, time] = (*ptr)[name];
    ip               = ip_in;
    time             = Clock::now();
    return std::move(ptr);
  });
}


std::string DnsCache::resolve(const std::string& name) const {
  auto const lock = *map_;
  if (auto const it = lock->find(name); it != lock->cend()) {
    return it->second.ip;
  }
  return {};
}


// O(DNS_CACHE_REC_LIMIT log DNS_CACHE_CLEANUP_BOUND)
void DnsCache::cleanup_if_needed(DnsCache::HashMap& map) const {
  assert(map.size() <= cache_size_);
  if (map.size() < cache_size_) {
    return;
  }

  using It                  = HashMap::const_iterator;
  auto constexpr queue_comp = [](It lhs, It rhs) noexcept {
    return lhs->second.timestamp < rhs->second.timestamp;
  };
  std::vector<It> mem;
  mem.reserve(cache_size_ - cache_limit_ + 1);
  std::priority_queue<It, decltype(mem), decltype(queue_comp)> queue{queue_comp, std::move(mem)};

  for (HashMap::const_iterator::difference_type i = 0; i < static_cast<HashMap::const_iterator::difference_type>(map.size()); ++i) {
    queue.push(std::next(map.cbegin(), i));
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
