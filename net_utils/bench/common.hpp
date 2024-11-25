#pragma once

#include <benchmark/benchmark.h>


namespace nut {


std::size_t constexpr rw_rel_multi = 100'000;
auto constexpr rel_range_default   = {1. / 10, 5. / 10, 9. / 10};


template<typename Base_>
class ThreadedRWBench
    : public benchmark::Fixture
    , public Base_ {
public:
  void bench_bind(benchmark::State& state, std::size_t r_idx, std::size_t w_idx, std::size_t rw_rel_idx) noexcept {
    this->set_rw_relation(static_cast<float>(state.range(rw_rel_idx)) / rw_rel_multi);
    state.counters["rs"] = this->readers;
    state.counters["ws"] = this->writers;

    this->read_iters            = state.range(r_idx);
    state.counters["r_rate"] = benchmark::Counter(this->read_iters / this->readers, benchmark::Counter::kIsRate);
    state.counters["r_it"]   = this->read_iters;

    this->write_iters            = state.range(w_idx);
    state.counters["w_rate"] = benchmark::Counter(this->write_iters / this->writers, benchmark::Counter::kIsRate);
    state.counters["w_it"]   = this->write_iters;
  }
};


}// namespace nut
