#include <chrono>
#include <raft/lib.hpp>
#include <raft/testing/definitions.hpp>

using namespace raft::testing;

int main(void){
    //fake a machine with 2 siblings and id 1
    min_machine m1({1,2,3}, 1);

    // tell the machine that 1 second has passed it should promote to candidate and go ask for votes
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
        std::cerr << "candidate's first action wasn't requesting a vote" << std::endl;
        std::cerr << "candidate's first action was: " << raft::display_action_variant(act.value().get_variant()) << std::endl;
        std::cerr << "candidate's queue was: " << std::endl;
        for(auto q : m1.queue()){
            std::cerr << "\t" << raft::display_action_variant(q.get_variant())<< "\t" << q.get_target() << std::endl;
        }
        std::exit(1);
    }

    //make sure the second action is to request a vote from the other sibling
    act = m1.crank_machine(std::chrono::milliseconds(1));

    if(m1.what_mode() != raft::candidate){
        std::cerr << "didn't remain a candidate" << std::endl;
        std::exit(1);
    }
    
    if(!act.has_value()){
        std::cerr << "candidate only asked for 1 vote" << std::endl;
        std::exit(1);
    }

    if(act.value().get_variant() != raft::io_action_variants::request_vote){
        std::cerr << "candidate's second action wasn't requesting a vote" << std::endl;
        std::cerr << "candidate's second action was: " << raft::display_action_variant(act.value().get_variant()) << std::endl;
        std::cerr << "candidate's queue was: " << std::endl;
        for(auto q : m1.queue()){
            std::cerr << "\t" << raft::display_action_variant(q.get_variant())<< "\t" << q.get_target() << std::endl;
        }
        std::exit(1);
    }

    // tell it that both siblings accepted it as leader
    m1.ack_rpc(2, raft::io_action_variants::request_vote, true, 1);
    m1.ack_rpc(3, raft::io_action_variants::request_vote, true, 1);

    m1.crank_machine(std::chrono::milliseconds(1));
    if(m1.what_mode() != raft::mode::leader){
        std::cerr << "candidate didn't promote to leader after all siblings voted" << std::endl;
        std::exit(1);
    }

    return 0;
}
