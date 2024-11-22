#pragma once

#include <net_utils/DnsCacheImpl.hpp>
#include <net_utils/Singleton.hpp>


namespace nut {


class DnsCache final : public Singleton<DnsCache, SingletonLivetimeMode::Global> {
public:
  static void update(const std::string& name, const std::string& ip) {
    instance()->p_.update(name, ip);
  }


  [[nodiscard]] static std::string resolve(const std::string& name) {
    return instance()->p_.resolve(name);
  }

private:
  template<typename... Args>
  explicit DnsCache(Args&&... args)
      : p_{std::forward<Args>(args)...} {
  }

private:
  DnsCacheImpl p_{};

  friend Singleton;
};


}// namespace nut