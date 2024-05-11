#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <boost/asio.hpp>
#include <raft/lib.hpp>


enum actions{add, remove_elem};
class table_action:raft::base_action{
    public:
    std::size_t idx;
    actions act;
     std::string describe() override{
         std::string ret;
         switch(act){
             case actions::add:
                ret += "a";
                break;
            case actions::remove_elem:
                ret += "r";
                break;
         }
         std::ostringstream n;
         n << idx;
         ret += n.str();
         return ret;
     }
     static table_action parse(std::string s){
         table_action ret;
         switch(s[0]){
             case 'a':
                ret.act = actions::add;
                break;
             case 'r':
                ret.act = actions::remove_elem;
                break;
         }
         std::istringstream i(s.substr(1));
         i >> ret.idx;
         return ret;
     }
};
class table: raft::base_state_machine<table_action>{
    std::vector<std::size_t> elems;
    public:
    table(): raft::base_state_machine<table_action>(){}
    void apply(table_action act) override {
        switch(act.act){
            case actions::add:
                if(act.idx > this->elems.size()){
                    std::size_t prev = elems.back();
                    for(std::size_t i = 0;i<act.idx;i++){
                        elems.push_back(prev+i+1);
                    }
                }
                break;
            case actions::remove_elem:
                elems.erase(elems.begin()+act.idx);
                break;
        }
    }
};
class nothing{};

std::map<raft::id_t, std::string> parse_sockets(){
    return std::map<raft::id_t, std::string>();
}

using machine_t = raft::state_machine<table_action, table, nothing>;

[[noreturn]]
void rpc_listener(std::string unix_socket, std::shared_ptr<machine_t> machine, std::mutex machine_mutex){
    // boost socket stuff copied from https://stackoverflow.com/a/37081979
    boost::asio::io_service io_service;
    using boost::asio::local::stream_protocol;

    stream_protocol::socket s(io_service);
    s.connect(unix_socket);

    while(true){
        
    }
}

int main(int argc, char *argv[]){

    std::vector<std::string> args(argv+1, argv+argc);
    std::string my_socket = args[0];
    std::map<raft::id_t, std::string> sibling_sockets = parse_sockets();
    std::set<raft::id_t> siblings = {1,2,3};
    std::shared_ptr<machine_t> machine = std::make_shared<machine_t>(siblings);
    std::mutex machine_mutex;
    std::chrono::time_point prev_time = std::chrono::steady_clock::now();
    while(true){
        std::chrono::time_point now_time = std::chrono::steady_clock::now();

        auto diff = now_time-prev_time;
        auto act = machine->crank_machine(std::chrono::duration_cast<std::chrono::milliseconds>(diff));
    }
    return 0;
}
