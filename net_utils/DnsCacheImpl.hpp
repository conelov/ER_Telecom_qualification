#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

#include <net_utils/RcuStorage.hpp>


namespace nut {


class DnsCacheImpl final {
public:
  ~DnsCacheImpl();

  DnsCacheImpl(std::size_t cache_size, std::size_t cache_cap);
  explicit DnsCacheImpl(std::size_t cache_size);
  DnsCacheImpl();

  void                      update(const std::string& name, const std::string& ip);
  [[nodiscard]] std::string resolve(const std::string& name);

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
  std::size_t const cache_size_;
  std::size_t const cache_cap_;
};


}// namespace nut
