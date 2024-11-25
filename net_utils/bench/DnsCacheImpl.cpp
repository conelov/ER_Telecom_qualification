#include <benchmark/benchmark.h>

#include <net_utils/aux/DnsCacheFixture.hpp>

#include <net_utils/bench/common.hpp>


using namespace nut;


namespace {


class DnsCacheBench : public ThreadedRWBench<aux::DnsCacheFixture> {
public:
  void SetUp(benchmark::State& state) override {
    bench_bind(state, 2, 3, 1);

    auto const c_size       = state.range(0);
    state.counters["cache"] = c_size;
    bind(c_size);
  }
};


void GenerateDependentArgs(benchmark::internal::Benchmark* b) {
  for (auto const cache_size : {10, 1'000, 10'000}) {
    for (auto const rw_rel : rel_range_default) {
      for (auto const wit : {10, 1'000, 10'000}) {
        for (auto const rit : {10, 10'000, 1'000'000}) {
          b->Args({cache_size, static_cast<std::int64_t>(std::round(rw_rel * rw_rel_multi)), rit, wit});
        }
      }
    }
  }
}


BENCHMARK_DEFINE_F(DnsCacheBench, general)(benchmark::State& state) {
  for (auto _ : state) {
    start();
  }
}


BENCHMARK_REGISTER_F(DnsCacheBench, general)
  ->Apply(GenerateDependentArgs)
  ->Unit(benchmark::kMillisecond)
  ->MeasureProcessCPUTime();


}// namespace
