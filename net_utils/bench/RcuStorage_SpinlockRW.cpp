#include <net_utils/bench/common.hpp>


using namespace nut;


RCU_BENCH(SpinlockRW, SpinlockRW<std::uint16_t>)
