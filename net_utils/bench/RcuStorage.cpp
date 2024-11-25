#include <net_utils/PriorityMutex.hpp>

#include <net_utils/aux/RcuStorageFixture.hpp>

#include <net_utils/bench/common.hpp>


using namespace nut;


namespace {


template<typename Mx_>
class RcuStorageBench : public ThreadedRWBench<aux::RcuStorageFixture<Mx_>> {
public:
  void SetUp(benchmark::State& state) override {
    this->bench_bind(state, 1, 2, 0);
    this->bind();
  }
};


void generate_dependent_args(benchmark::internal::Benchmark* b) {
  for (auto const rw_rel : rel_range_default) {
    for (auto const wit : {10'000, 100'000}) {
      for (auto const rit : {10'000, 100'000}) {
        b->Args({static_cast<std::int64_t>(std::round(rw_rel * rw_rel_multi)), rit, wit});
      }
    }
  }
}


}// namespace


BENCH_T(RcuStorageBench, priority_mutex, PriorityMutex<>);
BENCH_T(RcuStorageBench, std_mx, std::mutex);
