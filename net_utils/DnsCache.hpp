#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

#include <net_utils/RcuStorage.hpp>
#include <net_utils/Singleton.hpp>


namespace nut {


class DnsCache final : public Singleton<DnsCache, SingletonLivetimeMode::Global> {
public:
  ~DnsCache();

  static void                      update(const std::string& name, const std::string& ip);
  [[nodiscard]] static std::string resolve(const std::string& name);

private:
  DnsCache(std::size_t cache_limit, std::size_t cache_size);
  explicit DnsCache(std::size_t cache_limit = DNS_CACHE_REC_LIMIT);

private:
  using Clock = std::chrono::steady_clock;

  struct Rec final {
    std::string       ip;
    Clock::time_point timestamp;
  };

  using HashMap = std::unordered_map<std::string, Rec>;

private:
  void cleanup_if_needed(HashMap& map) const;

private:
  RcuStorage<HashMap> mutable map_;
  std::size_t const cache_limit_;
  std::size_t const cache_size_;

  friend Singleton<DnsCache, SingletonLivetimeMode::Global>;
};


}// namespace nut
