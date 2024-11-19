#include <net_utils/DnsCache.hpp>


namespace nut {


DnsCache::~DnsCache() = default;


DnsCache::DnsCache() {
//  data_.reserve(DNS_CACHE_REC_LIMIT);
}


void DnsCache::update_global(const std::string& name, const std::string& ip) {
  instance()->update(name, ip);
}


std::string DnsCache::resolve_global(const std::string& name) {
  return instance()->resolve(name);
}


void DnsCache::update(const std::string& name, const std::string& ip) {

}


std::string DnsCache::resolve(const std::string& name) const {
  return std::string();
}


}// namespace nut
