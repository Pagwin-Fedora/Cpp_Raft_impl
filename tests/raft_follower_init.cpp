#include <chrono>
#include <cstdlib>
#include <raft/lib.hpp>
#include <raft/testing/definitions.hpp>

using namespace raft::testing;

int main(void){
    // initialize a machine thinking it has 2 siblings
    min_machine m({1,2,3}, 1);
    
    // after just 10 milliseconds it shouldn't be trying to do anything yet
    auto act = m.crank_machine(std::chrono::milliseconds(10));
    if(act.has_value()){
        auto some_act = act.value();
        std::cerr << "shouldn't be doing anything yet" << std::endl;
        std::cerr << raft::display_action_variant(some_act.get_variant()) << '\t' << some_act.get_target() << std::endl;
        std::exit(1);
        
    }
    m.request_votes(1, 2, 0, 0);

    act = m.crank_machine(std::chrono::milliseconds(1));

    if(!act.has_value()){
        std::cerr << "machine isn't acknowledging a candidate's request for votes" << std::endl;
        std::exit(1);
    }
    auto some_act = act.value();
    if(some_act.get_variant() != raft::io_action_variants::acknowledge_rpc || some_act.get_target() != 2){
        std::cerr << "machine is doing something other than acknowleding request for votes for unclear reason" << std::endl;
        std::cerr << raft::display_action_variant(some_act.get_variant()) << '\t' << raft::display_id(some_act.get_target()) << std::endl;
        std::exit(1);
    }
    
    // leader with term 1 id 2(the one we just voted for) sending a heartbeat to us
    m.append_entries(1,2,0,0,{},0);

    act = m.crank_machine(std::chrono::milliseconds(1));

    if(!act.has_value()){
        std::cerr << "machine isn't acknowledging new leader" << std::endl;
        std::exit(1);
    }
    
    some_act = act.value();
    if(some_act.get_variant() != raft::io_action_variants::acknowledge_rpc || some_act.get_target() != 2){
        
    }
}
