#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

#include <net_utils/RcuStorage.hpp>
#include <net_utils/Singleton.hpp>


namespace nut {


class DnsCache final : public Singleton<DnsCache, SingletonLivetimeMode::Global> {
public:
  static void                      update(const std::string& name, const std::string& ip);
  [[nodiscard]] static std::string resolve(const std::string& name);

public:
  ~DnsCache();

private:
  DnsCache(std::size_t cache_limit, std::size_t cache_size);
  explicit DnsCache(std::size_t cache_limit);
  DnsCache();

private:
  using Clock = std::chrono::steady_clock;

  struct Rec final {
    std::string       ip;
    Clock::time_point timestamp;
  };

  using HashMap = std::unordered_map<std::string, Rec>;
  using Storage = RcuStorage<HashMap>;

private:
  void cleanup_if_needed(HashMap& map) const;

private:
  Storage           map_;
  std::size_t const cache_limit_;
  std::size_t const cache_size_;

  friend Singleton<DnsCache, SingletonLivetimeMode::Global>;
};


}// namespace nut
