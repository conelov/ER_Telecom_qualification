#pragma once

#include <benchmark/benchmark.h>

#include <net_utils/aux/SpinlockRWFixture.hpp>


namespace nut {


std::size_t constexpr rw_rel_multi = 100'000;
auto constexpr rw_rel_range        = {9. / 10, 5. / 10, 1. / 10};


template<typename Mx_>
class SharedMxBench
    : public benchmark::Fixture
    , public aux::SpinlockRWFixture<Mx_> {
public:
  void SetUp(benchmark::State& state) override {
    this->set_rw_relation(static_cast<float>(state.range(0)) / rw_rel_multi);
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


inline void shared_mx_bench_generate_dependent_args(benchmark::internal::Benchmark* b) {
  for (auto const rw_rel : rw_rel_range) {
    for (auto const rit : {10'000, 1'000'000}) {
      for (auto const wit : {1'000, 100'000}) {
        b->Args({static_cast<std::int64_t>(std::round(rw_rel * rw_rel_multi)), rit, wit});
      }
    }
  }
}


}// namespace nut


#define SHARED_MX_BENCH(name, type)                                                  \
  BENCHMARK_TEMPLATE_DEFINE_F(SharedMxBench, name, type)(benchmark::State & state) { \
    for (auto _ : state) {                                                           \
      start();                                                                       \
    }                                                                                \
  }                                                                                  \
  BENCHMARK_REGISTER_F(SharedMxBench, name)                                          \
    ->Apply(shared_mx_bench_generate_dependent_args)                                 \
    ->Unit(benchmark::kNanosecond)                                                   \
    ->MeasureProcessCPUTime();
