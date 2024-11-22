#include <cassert>
#include <cmath>
#include <queue>

#include <net_utils/DnsCacheImpl.hpp>


namespace nut {
namespace {
namespace _ {


constexpr auto kCacheCap = 9. / 10;


}// namespace _
}// namespace


DnsCacheImpl::~DnsCacheImpl() = default;


DnsCacheImpl::DnsCacheImpl(std::size_t cache_size, std::size_t cache_cap)
    : map_{std::make_shared<HashMap>()}
    , cache_size_{cache_size}
    , cache_cap_{cache_cap} {
  assert(cache_cap_ >= cache_size_);
}


DnsCacheImpl::DnsCacheImpl(std::size_t cache_size)
    : DnsCacheImpl{
        cache_size,
        cache_size + static_cast<std::size_t>(std::ceil(std::pow(cache_size, _::kCacheCap)))// heuristic predict cap
      } {
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


void DnsCacheImpl::cleanup_if_needed(HashMap& map) const {
  if (map.size() <= cache_cap_) {
    return;
  }

  // O(size * log(size - cap))
  using It                  = HashMap::const_iterator;
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


}// namespace nut
