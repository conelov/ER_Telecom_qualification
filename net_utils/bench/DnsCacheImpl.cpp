#include <benchmark/benchmark.h>

#include <net_utils/aux/DnsCacheFixture.hpp>

#include <net_utils/bench/common.hpp>


using namespace nut;


namespace {


template<typename>
class DnsCacheBench;


template<DnsCacheImplType type>
class DnsCacheBench<aux::DnsCacheImplTypeC<type>> : public ThreadedRWBench<aux::DnsCacheFixture<type>> {
public:
  void SetUp(benchmark::State& state) override {
    this->bench_bind(state, 1, 2, 0);

    auto const c_size       = state.range(3);
    state.counters["cache"] = c_size;
    this->bind(c_size);
  }
};


void generate_dependent_args(benchmark::internal::Benchmark* b) {
  for (auto const cache_size : {1'000, 10'000, 100'000}) {
    for (auto const rw_rel : rel_range_default) {
      for (auto const wit : {100, 1'000}) {
        for (auto const rit : {100, 10'000}) {
          b->Args({static_cast<std::int64_t>(std::round(rw_rel * rw_rel_multi)), rit, wit, cache_size});
        }
      }
    }
  }
}


}// namespace

BENCH_T(DnsCacheBench, lru_std_mx, aux::DnsCacheImplTypeC<DnsCacheImplType::lru_std_mx>);
BENCH_T(DnsCacheBench, lru_priority_mutex, aux::DnsCacheImplTypeC<DnsCacheImplType::lru_priority_mutex>);

// BENCH_T(DnsCacheBench, lru_lf, aux::DnsCacheImplTypeC<DnsCacheImplType::lru_priority_mutex>);

BENCH_T(DnsCacheBench, rcu_std_mx, aux::DnsCacheImplTypeC<DnsCacheImplType::rcu_std_mx>);
BENCH_T(DnsCacheBench, rcu_priority_mutex, aux::DnsCacheImplTypeC<DnsCacheImplType::rcu_priority_mutex>);
