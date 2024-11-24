#pragma once

#include <optional>
#include <vector>

#include <net_utils/RcuStorage.hpp>
#include <net_utils/utils.hpp>

#include <net_utils/aux/MultiThreadedRWFixture.hpp>


namespace nut::aux {


template<typename RcuMx_>
class RcuStorageFixture : public MultiThreadedRWFixture {
private:
  using ArrayCount = std::vector<std::uintmax_t>;
  using Storage    = RcuStorage<ArrayCount, RcuMx_>;

public:
  void up(std::size_t r_iters, std::size_t w_iters, std::size_t exclusive_writer_count) {
    assert(exclusive_writer_count <= writers);
    v_.emplace();

    MultiThreadedRWFixture::up(
      r_iters,
      w_iters,

      [this](auto...) {
        auto const array_ptr = v_->load();
        if (!array_ptr) {
          thread_pause();
          return;
        }
        for (auto const volatile& i : *array_ptr) {
          auto volatile dummy = i;
          ++dummy;
        }
      },

      [this, exclusive_writer_count](std::size_t w_index, auto...) {
        v_->modify(exclusive_writer_count <= w_index, [this, w_index](auto a_ptr) {
          if (!a_ptr) {
            a_ptr = std::make_shared<ArrayCount>(writers, 0);
          }
          ++a_ptr->at(w_index);
          return std::move(a_ptr);
        });
      });
  }


  void down() override {
    MultiThreadedRWFixture::down();
  }


  [[nodiscard]] Storage const& storage() const {
    return *v_;
  }

private:
  std::optional<Storage> v_;
};


}// namespace nut::aux