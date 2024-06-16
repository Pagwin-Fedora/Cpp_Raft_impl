#ifndef RAFT_RNG
#define RAFT_RNG
#include <cstdlib>
#include <ctime>
#include <optional>
namespace raft{
    
    class [[deprecated("Moving to stdlib named requirements RandomNumberEngine")]] base_rand{
        public:
        base_rand() = default;
        // needs to have a definition so... https://xkcd.com/221/
        virtual int operator()(){return 4;};
        virtual ~base_rand() {}
    };
    // use the C stdlib rand function for rng, use 0 in template for current time as seed
    template<int seed>
    class [[deprecated("Move to rng that isn't based on libc rand")]] crand:public base_rand{
        public:
        crand():base_rand(){
            std::srand(seed);
        }
        int operator()() override{
            return std::rand();
        }
        int seed_val(){
            return seed;
        }
    };

    //specialize for case where seed is runtime value
    template<>
    class crand<0>:public base_rand{
        private:
        int rseed;
        public:
        crand();
        crand(std::optional<unsigned int>);
        int operator()() override;
        int seed_val();
    };
}
#endif
