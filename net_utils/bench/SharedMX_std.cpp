#include <benchmark/benchmark.h>

#include <net_utils/bench/common.hpp>


using namespace nut;


SHARED_MX_BENCH(shared_mutex, std::shared_mutex)
