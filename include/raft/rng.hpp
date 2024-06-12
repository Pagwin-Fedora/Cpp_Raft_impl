#ifndef RAFT_RNG
#define RAFT_RNG
#include <cstdlib>
#include <ctime>
namespace raft{
    class base_rand{
        public:
        base_rand() = default;
        // needs to have a definition so... https://xkcd.com/221/
        virtual int operator()(){return 4;};
        virtual ~base_rand() {}
    };
    // use the C stdlib rand function for rng, use 0 in template for current time as seed
    template<int seed>
    class crand:public base_rand{
        public:
        crand():base_rand(){
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
