#ifndef RAFT_RNG
#define RAFT_RNG
#include <cstdlib>
#include <ctime>
namespace raft{
    class base_rand{
        base_rand() = default;
        virtual int operator()();
    };
    // use the C stdlib rand function for rng, use 0 in template for current time as seed
    template<int seed>
    class crand:base_rand{
        crand(){
            if constexpr (seed == 0){
                auto now = std::time(NULL);
                std::srand(static_cast<unsigned int>(now));
            }
            else{
                std::srand(seed);
            }
        }
        int operator()() override{
            return std::rand();
        }
        int seed_val(){
            return seed;
        }
    };
}
#endif
