#include <benchmark/benchmark.h>

#include <net_utils/aux/SpinlockRWFixture.hpp>

#include <net_utils/bench/common.hpp>


namespace {


using namespace nut;


std::size_t constexpr rw_rel_mutli = 100'000;


template<typename Mx_>
class SpinlockRWBench
    : public benchmark::Fixture
    , public aux::SpinlockRWFixture<Mx_> {
public:
  void SetUp(benchmark::State& state) override {
    this->set_rw_relation(static_cast<float>(state.range(0)) / rw_rel_mutli);
    state.counters["rs"] = this->readers;
    state.counters["ws"] = this->writers;


    auto const r_iters         = state.range(1);
    state.counters["rit_rate"] = benchmark::Counter(r_iters / this->readers, benchmark::Counter::kIsRate);
    state.counters["rit"]      = r_iters;

    auto const w_iters         = state.range(2);
    state.counters["wit_rate"] = benchmark::Counter(w_iters / this->writers, benchmark::Counter::kIsRate);
    state.counters["wit"]      = w_iters;

    this->up(r_iters, w_iters);
  }


  void TearDown(benchmark::State& state) override {
    this->down();
  }
};


void GenerateDependentArgs(benchmark::internal::Benchmark* b) {
  for (auto const rw_rel : rw_rel_range) {
    for (auto const rit : {10'000, 100'000, 1'000'000}) {
      for (auto const wit : {1'000, 10'000, 100'000}) {
        b->Args({static_cast<std::int64_t>(std::round(rw_rel * rw_rel_mutli)), rit, wit});
      }
    }
  }
}


BENCHMARK_TEMPLATE_DEFINE_F(SpinlockRWBench, shared_mutex, std::shared_mutex)(benchmark::State& state) {
  for (auto _ : state) {
    start();
  }
}


BENCHMARK_REGISTER_F(SpinlockRWBench, shared_mutex)
  ->Apply(GenerateDependentArgs)
  ->Unit(benchmark::kNanosecond)
  ->MeasureProcessCPUTime();


BENCHMARK_TEMPLATE_DEFINE_F(SpinlockRWBench, spinlock_rw, SpinlockRW)(benchmark::State& state) {
  for (auto _ : state) {
    start();
  }
}


BENCHMARK_REGISTER_F(SpinlockRWBench, spinlock_rw)
  ->Apply(GenerateDependentArgs)
  ->Unit(benchmark::kNanosecond)
  ->MeasureProcessCPUTime();


}// namespace
