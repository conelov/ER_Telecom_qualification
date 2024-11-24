#pragma once

#include <shared_mutex>
#include <string>
#include <mutex>
#include <string_view>

#include <net_utils/LruStorage.hpp>



namespace nut::aux {


class DnsCacheImplLRU {
public:
  ~DnsCacheImplLRU();
  DnsCacheImplLRU(std::size_t cache_size);

  void                      update(const std::string& name, const std::string& ip);
  [[nodiscard]] std::string resolve(const std::string& name) const;

private:
  LruStorage<std::string, std::string, std::string_view> mutable st_;
  std::shared_mutex mutable mx_;
};


} // namespace nut::aux

