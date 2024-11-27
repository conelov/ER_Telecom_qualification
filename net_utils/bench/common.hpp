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
    ->Apply(generate_dependent_args)


#define BENCH(fixture, name)                                    \
  BENCHMARK_DEFINE_F(fixture, name)(benchmark::State & state) { \
    for (auto _ : state) {                                      \
      start();                                                  \
    }                                                           \
  }                                                             \
  BENCHMARK_REGISTER_F(fixture, name)                           \
    ->Apply(generate_dependent_args)


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


  void post_stop() override {
    // state_->counters["r_rate"] = *this->read_rate;
    // state_->counters["w_rate"] = *this->write_rate;
  }

private:
  benchmark::State* state_ = nullptr;
};


}// namespace nut
