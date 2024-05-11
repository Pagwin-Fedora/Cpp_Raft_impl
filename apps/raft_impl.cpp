#include <boost/asio/buffer.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <chrono>
#include <cstddef>
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
class table_action:public raft::base_action{
    public:
    std::size_t idx;
    actions act;
     std::string describe() const override{
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
class table: public raft::base_state_machine<table_action>{
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
using io_t = raft::io_action<table_action, nothing>;

[[noreturn]]
void rpc_listener(std::string unix_socket, std::shared_ptr<machine_t> machine, std::mutex machine_mutex){
    // boost socket stuff copied from https://stackoverflow.com/a/37081979
    using boost::asio::local::stream_protocol;

    boost::asio::io_service io_service;
    stream_protocol::socket connection(io_service);
    stream_protocol::endpoint ep(unix_socket);
    stream_protocol::acceptor accept(io_service, ep);
    accept.listen(10);
    
    // I think this'll do what I want of accepting connections and handling them
    boost::asio::mutable_buffer buf;
    for(accept.accept(connection);;accept.accept(connection)){
        connection.receive(buf);
        std::string message(static_cast<char*>(buf.data()), static_cast<char*>(buf.data())+buf.size());
        std::istringstream parsing(message);
        std::string line;
        std::getline(parsing, line);
        // rpc_name: num_of_args
        // arg1
        // arg2
        // arg3
        std::string rpc_name;
        std::istringstream line1(line);
        std::getline(line1, rpc_name, ':');
        std::size_t arg_count;
        line1 >> arg_count;
        std::vector<std::string> args;
        for(std::size_t i = 0;i<arg_count; i++){
            std::string current_line;
            std::getline(parsing, current_line);
            args.push_back(std::move(current_line));
        }
        connection.close();
        //unlock after if/else chain
        machine_mutex.lock();
        if(rpc_name == "acknowledge_rpc"){
            raft::id_t ack_from = raft::parse_id(args[0]);
            raft::io_action_variants ack_of = raft::parse_action_variant(args[1]).value();
            bool successful = args[2] == "success";

            machine->ack_rpc(ack_from, ack_of, successful);
        }
        else if(rpc_name == "append_entries"){
            raft::term_t callee_term = raft::parse_term(args[0]);
            raft::id_t call_from = raft::parse_id(args[1]);
            raft::index_t prevIndex = raft::parse_index(args[2]);
            raft::term_t prevTerm = raft::parse_term(args[3]);
            std::vector<table_action> actions = raft::parse_actions<table_action>(args[4]);
            raft::index_t committedIndex = raft::parse_index(args[5]);

            machine->append_entries(callee_term, call_from, prevIndex, prevTerm, actions, committedIndex);
        }
        else if(rpc_name == "request_vote"){
            raft::term_t requester_term = raft::parse_term(args[0]);
            raft::id_t requester = raft::parse_id(args[1]);
            raft::index_t lastLogIndex = raft::parse_index(args[2]);
            raft::term_t lastLogTerm = raft::parse_term(args[3]);
            
            machine->request_votes(requester_term, requester, lastLogIndex, lastLogTerm);
        }
        machine_mutex.unlock();
    }
}

void perform_act(io_t action, std::map<raft::id_t, std::string> const& mapping){
    using raft::io_action_variants;
    std::ostringstream ss;
    raft::id_t target;
    ss << raft::display_action_variant(action.get_variant()) << ':';
    //switch statement is problematic for unknown but silly reasons
    #define CHECK(x) if(action.get_variant() == x)
    CHECK(io_action_variants::send_log){
        auto info = std::get<std::pair<std::vector<table_action>,raft::id_t>>(action.contents());
        auto log = std::get<0>(info);
        target = std::get<1>(info);
        ss << "1\n";
        ss << raft::display_actions(log);
    }
    else CHECK(io_action_variants::request_vote){
        auto info = std::get<raft::vote_request_state>(action.contents());
        target = info.send_target();
        ss << "4\n";
        ss << info.line_separated();
    }
    else CHECK(io_action_variants::acknowledge_rpc){
        auto info = std::get<raft::rpc_ack>(action.contents());
        ss << "3\n";
        ss << raft::display_id(info.ack_receiver);
        ss << raft::display_action_variant(info.ack_what);
        ss << info.successful;
    }
    // noop no domain actions
    else CHECK(io_action_variants::domain_action){return;}
    else {/*unreachable*/}
    std::string msg = ss.str();
    using boost::asio::local::stream_protocol;
    boost::asio::io_service io_service;
    stream_protocol::socket connection(io_service);
    connection.connect(mapping.at(target));

    boost::asio::const_buffer buf(msg.c_str(), msg.size());
    connection.send(buf);
    connection.close();
}

int main(int argc, char *argv[]){
    std::pair<bool, bool> a = std::make_pair(true, false);

    std::vector<std::string> args(argv+1, argv+argc);
    std::string my_socket = args[0];
    std::map<raft::id_t, std::string> sibling_sockets = parse_sockets();
    std::set<raft::id_t> siblings = {1,2,3};
    std::shared_ptr<machine_t> machine = std::make_shared<machine_t>(siblings);
    std::mutex machine_mutex;
    using std::chrono::steady_clock;
    std::chrono::time_point prev_time = steady_clock::now();
    while(true){
        std::chrono::time_point now_time = steady_clock::now();

        auto diff = now_time-prev_time;
        machine_mutex.lock();
        auto act = machine->crank_machine(std::chrono::duration_cast<std::chrono::milliseconds>(diff));
        machine_mutex.unlock();

        prev_time = now_time;
        if(!act.has_value()) continue;
        
        perform_act(act.value(), sibling_sockets);
    }
    return 0;
}
