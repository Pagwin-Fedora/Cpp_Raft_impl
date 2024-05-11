#include <raft/lib.hpp>
#include <sstream>
#include <optional>

raft::id_t raft::parse_id(std::string const& s){
    std::istringstream ss(s);
    raft::id_t ret;
    ss >> ret;
    return ret;
}
std::optional<raft::io_action_variants> raft::parse_action_variant(std::string const& s){

    if(s == "send_log")return raft::io_action_variants::send_log;
    if(s == "request_vote") return raft::io_action_variants::request_vote;
    if(s == "acknowledge_rpc") return raft::io_action_variants::acknowledge_rpc;
    if(s == "domain_action") return raft::io_action_variants::domain_action;

    return std::nullopt;
}
raft::term_t raft::parse_term(std::string const& s){
    std::istringstream ss(s);
    raft::term_t ret;
    ss >> ret;
    return ret;

}
raft::index_t raft::parse_index(std::string const& s){
    std::istringstream ss(s);
    raft::index_t ret;
    ss >> ret;
    return ret;

}

raft::term_t raft::base_action::get_term(){
    return this->term;
}

std::string raft::display_id(raft::id_t const& id){
    std::ostringstream ss;
    ss << id;
    return ss.str();
}
std::string raft::display_action_variant(raft::io_action_variants const& variant){
    switch(variant){
        case raft::io_action_variants::send_log:
            return "send_log";
        case raft::io_action_variants::request_vote:
            return "request_vote";
        case raft::io_action_variants::domain_action:
            return "domain_action";
        case raft::io_action_variants::acknowledge_rpc:
            return "acknowledge_rpc";
    }
}
