#include <algorithm>
#include <chrono>
#include <raft/lib.hpp>
#include <raft/testing/definitions.hpp>

using namespace raft::testing;

void handle_rpc(std::vector<min_machine<true>>& machines, raft::io_action<min_action<true>,nothing> action){
    auto target = machines[action.get_target()-1];

    #define CHECK(x) if(action.get_variant() == (x))
    CHECK(raft::io_action_variants::send_log){
        auto specific = std::get<raft::send_log_state<min_action<true>>>(action.contents());
        target.append_entries(specific.leaderTerm, specific.leaderId, specific.prevLogIndex, specific.prevLogTerm, specific.actions, specific.committed);
    }
    CHECK(raft::io_action_variants::request_vote){
        auto specific = std::get<raft::vote_request_state>(action.contents());
        target.request_votes(specific.candidate_term, specific.candidate, specific.lastLogIndex, specific.lastLogTerm);

    }
    CHECK(raft::io_action_variants::acknowledge_rpc){
        auto specific = std::get<raft::rpc_ack>(action.contents());
        target.ack_rpc(specific.my_id, specific.ack_what, specific.successful, action.get_term());
    }
}

int main(void){
    // looping a million times to brute force any rng out rather than having a flaky test
    for(std::size_t counter = 0; counter < 1000000L;counter++){


    std::vector<min_machine<true>> machines;
    std::set<raft::id_t> ids = {1,2,3,4,5};
    for(auto id:ids){
        machines.push_back(min_machine<true>(ids,id));
    }
    bool success = false;
    for(int i = 0;i<2000;i++){
        std::vector<raft::io_action<min_action<true>,nothing>> actions;
        // batching cranks and handle_rpc calls to kinda simulate latency
        for(auto& machine:machines){
            auto crank = machine.crank_machine(std::chrono::milliseconds(1));
            if(crank.has_value()) actions.push_back(crank.value());
        }
        for(auto& action:actions){
            handle_rpc(machines, action);
        }

        success = std::any_of(machines.begin(), machines.end(), [](auto machine){
            return machine.what_mode() == raft::mode::leader;
        });

        if(success) break;
    }
    if(!success){
        std::cerr << "Failed to elect a leader in 2 simulated seconds" << std::endl;
        std::exit(1);
    }

    }
    return 0;
}
