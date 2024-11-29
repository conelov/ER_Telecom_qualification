#include <net_utils/PriorityMutex.hpp>

#include <net_utils/aux/PriorityMutexFixture.hpp>

#include <net_utils/bench/common.hpp>


using namespace nut;


namespace {


template<typename Mx_>
class PriorityMutexBench : public ThreadedRWBench<aux::PriorityMutexFixture<Mx_>> {
public:
  void SetUp(benchmark::State& state) override {
    this->bench_bind(state, 1, 2, 0);
    this->bind();
  }
};


void generate_dependent_args(benchmark::internal::Benchmark* b) {
  for (auto const rw_rel : rel_range_default) {
    for (auto const wit : {10'000}) {
      for (auto const rit : {10'000}) {
        b->Args({static_cast<std::int64_t>(std::round(rw_rel * rw_rel_multi)), rit, wit});
      }
    }
  }
}


}// namespace


BENCH_T(PriorityMutexBench, std_mx, std::shared_mutex);
BENCH_T(PriorityMutexBench, priority_mutex, PriorityMutex);
