#pragma once

#include <benchmark/benchmark.h>

#include <net_utils/aux/RcuStorageFixture.hpp>
#include <net_utils/aux/SpinlockRWFixture.hpp>


#define BENCH(fixture, name, type, args_gen)                                   \
  BENCHMARK_TEMPLATE_DEFINE_F(fixture, name, type)(benchmark::State & state) { \
    for (auto _ : state) {                                                     \
      start();                                                                 \
    }                                                                          \
  }                                                                            \
  BENCHMARK_REGISTER_F(fixture, name)                                          \
    ->Apply(args_gen)                                                          \
    ->Unit(benchmark::kNanosecond)                                             \
    ->MeasureProcessCPUTime();


namespace nut {


std::size_t constexpr rw_rel_multi = 100'000;
auto constexpr rw_rel_range        = {9. / 10, 5. / 10, 1. / 10};


template<typename Base_>
class ThreadedRWBench
    : public benchmark::Fixture
    , public Base_ {
public:
  void SetUp(benchmark::State& state) override {
    this->set_rw_relation(static_cast<float>(state.range(0)) / rw_rel_multi);
    state.counters["rs"] = this->readers;
    state.counters["ws"] = this->writers;


    auto const r_iters       = state.range(1);
    state.counters["r_rate"] = benchmark::Counter(r_iters / this->readers, benchmark::Counter::kIsRate);
    state.counters["r_it"]   = r_iters;

    auto const w_iters       = state.range(2);
    state.counters["w_rate"] = benchmark::Counter(w_iters / this->writers, benchmark::Counter::kIsRate);
    state.counters["w_it"]   = w_iters;
  }


  void TearDown(benchmark::State& state) override {
    cast()->down();
  }


  void bench_SetUp(benchmark::State& state) noexcept {
    ThreadedRWBench::SetUp(state);
  }


  Base_* cast() noexcept {
    return static_cast<Base_*>(this);
  }
};


template<typename Mx_>
class SharedMxBench : public ThreadedRWBench<aux::SpinlockRWFixture<Mx_>> {
public:
  void SetUp(benchmark::State& state) override {
    this->bench_SetUp(state);
    this->cast()->up(state.range(1), state.range(2));
  }
};


inline void SharedMxBench_generate_dependent_args(benchmark::internal::Benchmark* b) {
  for (auto const rw_rel : rw_rel_range) {
    for (auto const rit : {10'000, 1'000'000}) {
      for (auto const wit : {1'000, 100'000}) {
        b->Args({static_cast<std::int64_t>(std::round(rw_rel * rw_rel_multi)), rit, wit});
      }
    }
  }
}


}// namespace nut


#define SHARED_MX_BENCH(name, type) \
  BENCH(SharedMxBench, name, type, SharedMxBench_generate_dependent_args)


namespace nut {


template<typename Mx_>
class RcuBench : public ThreadedRWBench<aux::RcuStorageFixture<Mx_>> {
public:
  void SetUp(benchmark::State& state) override {
    this->bench_SetUp(state);

    auto const w_we_rel   = static_cast<float>(state.range(3)) / rw_rel_multi;
    state.counters["wes"] = std::round(this->writers * w_we_rel);

    this->cast()->up(state.range(1), state.range(2), w_we_rel);
  }
};


inline void RcuBench_generate_dependent_args(benchmark::internal::Benchmark* b) {
  for (auto const rw_rel : rw_rel_range) {
    for (auto const rit : {10'000 / 4, 1'000'000 / 4}) {
      for (auto const wit : {1'000 / 4, 100'000 / 4}) {
        for (auto const w_we_rel : rw_rel_range) {
          b->Args({static_cast<std::int64_t>(std::round(rw_rel * rw_rel_multi)), rit, wit, static_cast<std::int64_t>(std::round(w_we_rel * rw_rel_multi))});
        }
      }
    }
  }
}


}// namespace nut


#define RCU_BENCH(name, type) \
  BENCH(RcuBench, name, type, RcuBench_generate_dependent_args)
