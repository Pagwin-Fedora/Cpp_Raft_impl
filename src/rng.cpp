#include <raft/rng.hpp>


raft::crand<0>::crand():base_rand(){
    auto now = std::time(NULL);
    std::srand(static_cast<unsigned int>(now));
}
raft::crand<0>::crand(std::optional<unsigned int> seed): base_rand(){
    if(!seed.has_value()) return;
    this->rseed = seed.value();
    std::srand(this->rseed);
}
int raft::crand<0>::operator()(){
    return std::rand();
}
int raft::crand<0>::seed_val(){
    return this->rseed;
}
