#include <chrono>
#include <raft/lib.hpp>
#include <raft/testing/definitions.hpp>
#include <ratio>

using namespace raft::testing;

int main(void){
    min_machine m1({1,2,3}, 1);
    min_machine m2({1,2,3}, 2);
    min_machine m3({1,2,3}, 3);
    auto act = m1.crank_machine(std::chrono::milliseconds(1000));
    if(m1.what_mode() != raft::candidate){
        std::cerr << "didn't promote to candidate within a second" << std::endl;
        std::exit(1);
    }
    
    if(!act.has_value()){
        std::cerr << "candidate did nothing" << std::endl;
        std::exit(1);
    }

    if(act.value().get_variant() != raft::io_action_variants::request_vote){
        std::cerr << "candidate's action wasn't requesting a vote" << std::endl;
        std::exit(1);
    }

    m1.ack_rpc(2,raft::io_action_variants::request_vote, true, 1);
    m1.ack_rpc(3, raft::io_action_variants::request_vote, true, 1);

    m1.crank_machine(std::chrono::milliseconds(1));
 
    if(m1.what_mode() != raft::mode::leader){
        std::cerr << "candidate didn't promote to leader after all siblings voted" << std::endl;
        std::exit(1);
    }

    return 0;
}
