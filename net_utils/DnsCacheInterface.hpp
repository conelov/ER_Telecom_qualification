#pragma once

#include <string>


namespace nut {


class DnsCacheInterface {
public:
  virtual void        update(const std::string& name, const std::string& ip) = 0;
  virtual std::string resolve(const std::string& name) const                 = 0;
};


}// namespace nut