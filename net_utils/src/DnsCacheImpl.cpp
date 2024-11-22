#include <cassert>
#include <cmath>
#include <queue>

#include <net_utils/DnsCacheImpl.hpp>


namespace nut {
namespace {
namespace _ {


constexpr auto kCacheSize = 9. / 10;


}// namespace _
}// namespace


DnsCacheImpl::~DnsCacheImpl() = default;


DnsCacheImpl::DnsCacheImpl(std::size_t cache_limit, std::size_t cache_size)
    : map_{std::make_shared<HashMap>()}
    , cache_limit_{cache_limit}
    , cache_size_{cache_size} {
  assert(cache_size > cache_limit);
}


DnsCacheImpl::DnsCacheImpl(std::size_t cache_limit)
    : DnsCacheImpl{cache_limit, cache_limit + static_cast<std::size_t>(std::ceil(std::pow(cache_limit, _::kCacheSize)))} {
}


DnsCacheImpl::DnsCacheImpl()
    : DnsCacheImpl{DNS_CACHE_REC_LIMIT} {
}


void DnsCacheImpl::update(const std::string& name, const std::string& ip_in) {
  map_.modify([this, &name, &ip_in](auto ptr) {
    assert(ptr);
    auto& [ip, time] = (*ptr)[name];
    ip               = ip_in;
    time             = Clock::now();
    cleanup_if_needed(*ptr);
    return std::move(ptr);
  });
}


std::string DnsCacheImpl::resolve(const std::string& name) {
  if (auto const lock = *map_) {
    if (auto const it = lock->find(name); it != lock->cend()) {
      return it->second.ip;
    }
  }
  return {};
}


// O(size log limit)
void DnsCacheImpl::cleanup_if_needed(HashMap& map) const {
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
