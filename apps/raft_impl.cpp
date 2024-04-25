#include <iostream>
#include <vector>
#include <string>
#include <raft/lib.hpp>

class table_action:raft::base_action{

};
class table{};
class nothing{};

int main(int argc, char *argv[]){
    std::vector<std::string> args(argv+1, argv+argc);
    
    for(auto& arg:args) std::cout << arg << std::endl;


    raft::state_machine<table_action, table, nothing> machine;

    return 0;
}
