#pragma once

#include <numeric>

#include <benchmark/benchmark.h>


#define BENCH_T(fixture, name, type)                                           \
  BENCHMARK_TEMPLATE_DEFINE_F(fixture, name, type)(benchmark::State & state) { \
    for (auto _ : state) {                                                     \
      iteration();                                                             \
    }                                                                          \
  }                                                                            \
  BENCHMARK_REGISTER_F(fixture, name)                                          \
    ->Apply(generate_dependent_args)                                           \
    ->Unit(benchmark::kMillisecond)                                            \
    ->MeasureProcessCPUTime();


#define BENCH(fixture, name)                                    \
  BENCHMARK_DEFINE_F(fixture, name)(benchmark::State & state) { \
    for (auto _ : state) {                                      \
      start();                                                  \
    }                                                           \
  }                                                             \
  BENCHMARK_REGISTER_F(fixture, name)                           \
    ->Apply(generate_dependent_args)                            \
    ->Unit(benchmark::kMillisecond)                             \
    ->MeasureProcessCPUTime();


namespace nut {


std::size_t constexpr rw_rel_multi = 100'000;
auto constexpr rel_range_default   = {9. / 10, 5. / 10, 1. / 10};


template<typename Base_>
class ThreadedRWBench
    : public benchmark::Fixture
    , public Base_ {
public:
  void bench_bind(benchmark::State& state, std::size_t r_idx, std::size_t w_idx, std::size_t rw_rel_idx) noexcept {
    state_ = &state;

    this->set_rw_relation(static_cast<float>(state.range(rw_rel_idx)) / rw_rel_multi);
    state.counters["rs"] = this->readers;
    state.counters["ws"] = this->writers;

    this->read_iters       = state.range(r_idx);
    state.counters["r_it"] = this->read_iters;

    this->write_iters      = state.range(w_idx);
    state.counters["w_it"] = this->write_iters;
  }


  void pre_stop() override {
    if (this->payloads().empty()) {
      return;
    }
    IterativeAverage<std::chrono::duration<double, std::micro>> read_rate;
    IterativeAverage<std::chrono::duration<double, std::micro>> write_rate;
    for (auto const& p : this->payloads()) {
      assert(p->type == aux::MultiThreadedRWFixturePayloadMixin::reader
        || p->type == aux::MultiThreadedRWFixturePayloadMixin::writer);
      (p->type == aux::MultiThreadedRWFixturePayloadMixin::reader ? read_rate : write_rate) += p->rate->average();
    }
    state_->counters["r_rate_per_µs"]     = read_rate.average().count();
    state_->counters["r_rate_max_per_µs"] = read_rate.min_max().max.count();
    state_->counters["w_rate_per_µs"]     = write_rate.average().count();
    state_->counters["w_rate_max_per_µs"] = write_rate.min_max().max.count();
  }

private:
  benchmark::State* state_ = nullptr;
};


}// namespace nut
