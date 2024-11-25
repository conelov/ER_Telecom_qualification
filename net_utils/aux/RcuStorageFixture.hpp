#pragma once

#include <vector>

#include <net_utils/RcuStorage.hpp>

#include <net_utils/aux/MultiThreadedRWValuedFixture.hpp>


namespace nut::aux {


template<typename Mx_>
class RcuStorageFixture : public MultiThreadedRWValuedFixture<RcuStorage<std::vector<std::uintmax_t>, Mx_>> {
public:
  void bind() {
    using Base       = MultiThreadedRWValuedFixture<RcuStorage<std::vector<std::uintmax_t>, Mx_>>;
    this->value_ctor = [this](auto& opt) {
      opt.emplace(std::make_shared<typename Base::ValueOpt::value_type::value_type>(this->writers, 0));
    };

    Base::bind(
      [this](std::size_t idx, auto...) {
        auto const a_ptr    = this->value->load();
        auto volatile dummy = a_ptr->at(idx % this->writers);
        ++dummy;
      },
      [this](std::size_t idx, auto...) {
        this->value->modify([idx](auto a_ptr) {
          ++a_ptr->at(idx);
          return a_ptr;
        });
      });
  }
};


}// namespace nut::aux