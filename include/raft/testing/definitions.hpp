#include <raft/lib.hpp>
#include <set>
#include <string>

namespace raft{
    namespace testing{
        class nothing{};
        template<bool all_equal>
        class min_action: public raft::base_action{
            public:
            min_action():raft::base_action(){}
            std::string describe() const override{
                return "";
            }
            static min_action parse(std::string s){
                return min_action();
            }
            friend bool operator==(min_action const& lhs, min_action const& rhs){
                return all_equal;
            }
            friend bool operator!=(min_action const& lhs, min_action const& rhs){
                return !(lhs == rhs);
            }
        };
        template<bool all_actions_equal>
        using min_state_machine = raft::base_state_machine<min_action<all_actions_equal>>;
        template<bool all_actions_equal>
        class min_machine: public raft::state_machine<min_action<all_actions_equal>, min_state_machine<all_actions_equal>, nothing> {
            public:
            min_machine(std::set<raft::id_t> other, raft::id_t me): raft::state_machine<min_action<all_actions_equal>, min_state_machine<all_actions_equal>, nothing>(other, me){}
            raft::mode what_mode(){
                return this->currentState;
            }
            std::deque<raft::io_action<min_action<all_actions_equal>, nothing>>const& queue(){
                return this->needed_actions;
            }
        };
    }
}
