#include <benchmark/benchmark.h>

#include <net_utils/bench/common.hpp>


using namespace nut;


SHARED_MX_BENCH(SpinlockRW, SpinlockRW<std::uint16_t>)
