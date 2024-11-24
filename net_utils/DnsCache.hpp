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
  DnsCache(std::size_t capacity)
      : p_{capacity} {
  }

private:
  aux::DnsCacheImplLRU p_;

  friend Singleton;
};


}// namespace nut