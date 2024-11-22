#include <chrono>
#include <thread>

#include <benchmark/benchmark.h>

#include <net_utils/aux/DnsCacheFixture.hpp>


using namespace nut;


namespace {


class DnsCacheBench
    : public benchmark::Fixture
    , public DnsCacheFixture {
public:
  void SetUp(benchmark::State& state) override {
    std::size_t const cache_size  = state.range(0);
    std::size_t const cache_limit = state.range(1);

    iterations = state.range(2);
    up(cache_limit, cache_size);
  }

  void TearDown(benchmark::State& state) override {
    down();
    // state.SetComplexityN(iterations);
  }
};


void GenerateDependentArgs(benchmark::internal::Benchmark* b) {
  std::size_t constexpr size_lo = 8;
  for (auto const size : benchmark::CreateRange(size_lo, 128, 8)) {
    for (auto const lim_div : benchmark::CreateRange(2, size, size / (size_lo / 2))) {
      b->Args({size, size / lim_div, 1'000});
    }
  }
}


BENCHMARK_DEFINE_F(DnsCacheBench, general)(benchmark::State& state) {
  for (auto _ : state) {
    compute();
  }
}


BENCHMARK_REGISTER_F(DnsCacheBench, general)
  ->Apply(GenerateDependentArgs)
  ->Unit(benchmark::kMillisecond)
  ->MeasureProcessCPUTime();


}// namespace
