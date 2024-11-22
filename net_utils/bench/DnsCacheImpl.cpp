#include <chrono>
#include <thread>

#include <benchmark/benchmark.h>

#include <net_utils/aux/DnsCacheFixture.hpp>


using namespace nut;


namespace {


std::size_t constexpr rw_rel_mutli = 100'000;


class DnsCacheBench
    : public benchmark::Fixture
    , public DnsCacheFixture {
public:
  void SetUp(benchmark::State& state) override {
    std::size_t const cache_size = state.range(0);
    state.counters["size"]       = cache_size;

    std::size_t const cache_cap = state.range(1);
    state.counters["cap"]       = cache_cap;

    rw_relation          = static_cast<float>(state.range(3)) / rw_rel_mutli;
    state.counters["rs"] = readers();
    state.counters["ws"] = writers();

    iterations            = state.range(2);
    state.counters["its"] = iterations;

    state.counters["rate"] = benchmark::Counter(iterations, benchmark::Counter::kIsRate);

    up(cache_size, cache_cap);
  }

  void TearDown(benchmark::State& state) override {
    down();
    // state.SetComplexityN(iterations);
  }
};


void GenerateDependentArgs(benchmark::internal::Benchmark* b) {
  std::size_t constexpr size_lo = 8;
  for (auto const rw_rel : {9. / 10, 5. / 10, 1. / 10}) {
    for (auto const size : benchmark::CreateRange(size_lo, 2 << 12, 2 << 2)) {
      for (auto const cap : benchmark::CreateDenseRange(15,  30, 5)) {
        b->Args({size, static_cast<std::int64_t>(size * (cap / 10.)), size << 1, static_cast<std::int64_t>(std::round(rw_rel * rw_rel_mutli))});
      }
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
