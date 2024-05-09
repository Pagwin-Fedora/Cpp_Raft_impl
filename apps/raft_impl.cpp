#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <raft/lib.hpp>

class table_action:raft::base_action{

};
class table: raft::base_state_machine<table_action>{
    public:
    table(): raft::base_state_machine<table_action>(){}
};
class nothing{};

int main(int argc, char *argv[]){
    std::vector<std::string> args(argv+1, argv+argc);
    
    for(auto& arg:args) std::cout << arg << std::endl;

    std::set<raft::id_t> siblings = {1,2,3};
    raft::state_machine<table_action, table, nothing> machine(siblings);

    return 0;
}
