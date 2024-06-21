#ifndef RAFT_RNG
#define RAFT_RNG
#include <cstdlib>
#include <random>
namespace raft{
    // convenience method
    std::seed_seq make_seed_seq(std::size_t num_count);

    // make a vector of numbers to give to seed_seq to seed rng with
    std::vector<std::uint32_t> make_seeds(std::size_t count);

}
#endif
