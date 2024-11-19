#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_set>

#include <net_utils/DnsCacheInterface.hpp>
#include <net_utils/Singleton.hpp>


namespace nut {


class DnsCache final : public DnsCacheInterface
    , public Singleton<DnsCache, SingletonLivetimeMode::Global> {
public:
  static void                      update_global(const std::string& name, const std::string& ip);
  [[nodiscard]] static std::string resolve_global(const std::string& name);

public:
  ~DnsCache();
  DnsCache();

  void                      update(const std::string& name, const std::string& ip) override;
  [[nodiscard]] std::string resolve(const std::string& name) const override;

private:
  struct Node final {
    std::string       name;
    std::string       ip;
    std::atomic<bool> valid = true;
  };

private:
  static auto constexpr node_hash = [](Node const& n) -> std::uint64_t {
    if (!n.valid) {
      return 0;
    }
    return std::hash<decltype(n.name)>{}(n.name);
  };


  static auto constexpr node_cmp = [](Node const& lhs, Node const& rhs) {
//    return
  };

private:
  std::unordered_set<Node> data_;


  friend Singleton<DnsCache, SingletonLivetimeMode::Global>;
};


}// namespace nut
