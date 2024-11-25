#include <net_utils/DnsCacheImpl.hpp>


namespace nut::aux {


DnsCacheImplLRU::~DnsCacheImplLRU() = default;


DnsCacheImplLRU::DnsCacheImplLRU(std::size_t cache_size)
    : st_{cache_size} {
}


void DnsCacheImplLRU::update(const std::string& name, const std::string& ip) {
  {
    std::lock_guard const lock{mx_};
    st_.put(name, ip);
  }
}


std::string DnsCacheImplLRU::resolve(const std::string& name) const {
  {
    std::lock_guard const lock{mx_};
    if (auto const out_ptr = st_.get(name)) {
      return *out_ptr;
    } else {
      return {};
    }
  }
}


}// namespace nut::aux
