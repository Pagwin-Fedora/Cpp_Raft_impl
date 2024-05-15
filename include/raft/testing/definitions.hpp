#include <raft/lib.hpp>
#include <set>
#include <string>

namespace raft{
    namespace testing{
        class nothing{};
        class min_action: public raft::base_action{
            public:
            min_action():raft::base_action(){}
            std::string describe() const override{
                return "";
            }
            static min_action parse(std::string s){
                return min_action();
            }
        };
        using min_state_machine = raft::base_state_machine<min_action>;
        class min_machine: public raft::state_machine<min_action, min_state_machine, nothing> {
            public:
            min_machine(std::set<raft::id_t> other, raft::id_t me): raft::state_machine<min_action, min_state_machine, nothing>(other, me){}
            raft::mode what_mode(){
                return this->currentState;
            }
        };
    }
}
