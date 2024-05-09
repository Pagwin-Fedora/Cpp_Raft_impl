#include <algorithm>
#include <chrono>
#include <cstdint>
#include <deque>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

// on page 4 there's a 1 page reference sheet https://raft.github.io/raft.pdf
// TODO: implement everything under Rules for servers
namespace raft{

    using id_t = std::uint64_t;
    using index_t = std::uint64_t;
    using term_t = std::uint64_t;

    // the base class of whatever actions can be applied to the state machine
    class base_action {
        public:
        // inverse of parse
        std::string describe();
        // parse a string to get the action desired back
        static base_action parse(std::string);
    };

    template<typename Action>
    class base_state_machine {
        static_assert(std::is_base_of_v<base_action, Action>);
        // this method should be overriden by the real implementation
        void apply(Action action){
            //noop for base implementation
        }
        public:
        base_state_machine();
    };
    enum mode{
        follower,
        candiate,
        leader
    };

    enum io_action_variants{
        send_log,
        request_vote,
        domain_action,
        acknowledge_rpc
    };
    struct rpc_ack{
        io_action_variants ack_what;
        id_t my_id;
        id_t ack_receiver;
        bool successful;
    };
    // Action is subclass of base_action which we can use for various things
    // DomainAction is a misc action that isn't related to Raft that needs to be taken by whatever is doing io
    // io_action is trying to be a tagged union telling the IO impl what to do
    template <typename Action, typename DomainAction>
    class io_action{
        static_assert(std::is_base_of_v<base_action, Action>,"ActionType must inherit from base_action to ensure it has associated state");
        
        using vote_request_state = id_t;
        using send_log_state = std::pair<std::vector<Action>, id_t>;
        using content = std::variant<send_log_state, DomainAction, rpc_ack, vote_request_state>;

        term_t sent_at;
        io_action_variants variant;
        content msg_contents;
        
        
        public:
        io_action(io_action_variants variant , content msg, term_t term):sent_at(term), msg_contents(msg), variant(variant){}
        io_action_variants const& get_variant(){return this->variant;}
        id_t const& get_target(){return this->target;}
        content const& contents(){return this->msg_contents;}
    };

    template<typename Action, typename InnerMachine, typename DomainAction>
    class state_machine{
        static_assert(std::is_base_of_v<base_action, Action>,"ActionType must inherit from base_action to ensure it has associated state");
        static_assert(std::is_base_of_v<base_state_machine<Action>, InnerMachine>, "The Inner State Machine needs to inherit from base_state_machine because C++ doesn't have interfaces");

        // normally I don't put _t after type names but id, index and term can easily be var names so clarification seemed useful

        id_t myId;
        std::uint64_t currentTerm;
        // value used to indicate the id of the node we're following, either candidate we voted for or current leader
        std::optional<id_t> following;
        std::set<id_t> siblings;
        std::map<id_t, index_t> replicatedIndices;
        std::vector<Action> log;
        index_t commitIndex;
        index_t lastApplied;
        mode currentState;
        std::deque<io_action<Action, DomainAction>> needed_actions;

        // implementation details for IO and state machine
        std::unordered_set<id_t> servers;
        InnerMachine log_result;
        void ack(io_action_variants act, bool success, id_t target){
            this->needed_actions.push_back(io_action<Action,DomainAction>(
                io_action_variants::acknowledge_rpc,
                rpc_ack{.my_id = this->myId, .ack_what = act, .successful = success, .ack_receiver = target}),
                this->currentTerm
            );
        }
        void send_log(std::vector<Action> log, id_t target){
            this->needed_actions.push_back(io_action<Action, DomainAction>(
                io_action_variants::send_log,
                std::make_pair(std::move(log), target),
                this->currentTerm
            ));
        }
        void request_vote(id_t target){
            this->needed_actions.push_back(io_action<Action, DomainAction>(
                io_action_variants::request_vote,
                target,
                this->currentTerm
            ));
        }
        public:
        state_machine(std::set<id_t> siblings): siblings(std::move(siblings)){}
        // I really don't like that templated functions need to go in headers but oh well
        // might be sensible to trim the arg count down via a struct or class which contains all this and builder pattern
        void append_entries(term_t term, id_t leaderId, index_t prevLogIndex, term_t prevLogTerm, std::vector<Action> const& entries, index_t leaderCommit, std::chrono::milliseconds time_passed) noexcept {
            if(term < this->currentTerm){
                //return std::make_pair(std::move(this->currentTerm), false);
                this->ack(io_action_variants::send_log, false, leaderId);
            }
            // keep a list of things we want IO to do when it pings us with crank again and apped this to that
            if(log.size() < prevLogIndex || std::get<0>(log[prevLogIndex]) != prevLogTerm){
                this->ack(io_action_variants::send_log, false, leaderId);
                //return std::make_pair(std::move(term), false);
            }

            auto first_to_remove = std::find_if(this->log.begin(), this->log.end(), [entries](Action const& myAction){
                    return std::any_of(entries.begin(),entries.end(), [myAction](Action const& leaderAction){
                        return myAction.idx == leaderAction.idx && myAction.term != leaderAction.term;
                    });
            });

            this->log.erase(first_to_remove, this->log.end());

            std::copy_if(entries.begin(), entries.end(), std::back_inserter(this->log), [this](Action& a){
                return std::none_of(this->log.begin(), this->log.end(), [a](Action& b){return a.idx != b.idx;});
            });
            if(leaderCommit > commitIndex) commitIndex = std::min(leaderCommit, std::max_element(this->log.begin(), this->log.end(), [](Action const& a, Action const& b){return a.idx < b.idx;}));

            // keep a list of things we want IO to do when it pings us with crank again and apped this to that
            //return std::make_pair(std::move(term), true);
            this->needed_actions.push_back(io_action<Action,DomainAction>(
                io_action_variants::acknowledge_rpc,
                rpc_ack{.my_id = this->myId, .ack_what = io_action_variants::send_log, .successful = false}),
                this->currentTerm
            );
        }

        void request_votes(term_t term, id_t candidateId, index_t lastLogIndex, term_t lastLogTerm, std::chrono::milliseconds time_passed) noexcept{
            if(term < this->currentTerm){
                //return std::make_pair(this->currentTerm, false);
                this->ack(io_action_variants::request_vote, false, candidateId);
            }

            if(!following.has_value() || following.value() == candidateId){

                bool got_vote = lastLogTerm >= std::max_element(this->log.begin(), this->log.end(), [](Action a, Action b){return a.term < b.term;});

                if(got_vote) this->votedFor.emplace(candidateId);
                // keep a list of things we want IO to do when it pings us with crank again and apped this to that
                this->ack(io_action_variants::request_vote, got_vote, candidateId);
                //return std::make_pair(std::move(term), got_vote);
            }
            else {
                // keep a list of things we want IO to do when it pings us with crank again and apped this to that
                this->ack(io_action_variants::request_vote, false, candidateId);
                //return std::make_pair(std::move(term), false);
            }
        }
        
        void ack_rpc(id_t ack_from, io_action_variants action, bool successful){
            switch(this->currentState){
                case mode::leader:
                    // handle case where acknowledging a log appending as well as when being demoted
                    if(!successful){
                        // we got demoted, swapping to follower
                        this->swap_follower();
                        return;
                    }
                    switch(action){
                        // we don't know anything about domain_actions so noop
                        case io_action_variants::domain_action:
                            break;
                        case io_action_variants::acknowledge_rpc:
                            //unreachable acknowledghing an acknowledge is nonsense
                            break;
                        case io_action_variants::send_log:
                            //TODO we can send logs with more than 1 entry so handle that correctly
                            this->replicatedIndices[ack_from]++;
                        break;
                        case raft::io_action_variants::request_vote:
                            // nothing to do they'll find out we're leader in the next regularly scheduled heartbeat
                        break;
                    }
                    break;
                case mode::candiate:
                    // handle finding out not being up to date as well as getting another vote
                    break;
                case mode::follower:
                    // ignore any acks they can only be from when we were in a different state
                    break;
            }
        }
        
        // change to follower state and remove io_actions which aren't ack
        void swap_follower(){
            auto follower_whitelist = [](io_action<Action, DomainAction> a){
                    return a.variant != io_action_variants::acknowledge_rpc && a.variant != io_action_variants::domain_action;
            };
            this->currentState = mode::follower;
            // convoloted while loop to remove anything that doesn't match follower_whitelist
            while(std::find_if(this->needed_actions.begin(), this->needed_actions.end(), follower_whitelist) != this->needed_actions.end()){
                this->needed_actions.erase(std::find_if(this->needed_actions.begin(),this->needed_actions.end(), follower_whitelist));
            }
        }
        
        // InputIt is an iterator over values of type Action
        // Put actions into queue to be committed if we're the leader and return the id of the leader if we aren't
        template<typename InputIt>
        std::optional<id_t> enqueue_actions(InputIt start, InputIt end) noexcept{
            static_assert(typeid(Action) == typeid(*start), "Iterator must iterate over values of type Action (can't be more specific due to this needing to be a string literal)");
            switch (this->currentState){
                case mode::leader:
                    std::move(start, end, std::back_inserter(this->log));
                    return std::nullopt;
                break;
                case mode::candiate:
                    // ???
                    break;
                case mode::follower:
                    return this->following;
                break;
            }
            return std::nullopt;
        }
        bool calling_election(){
            //TODO put the logic for whether or not we need to call an election here 
        }
        // method that tells the machine how long it's been since the last crank for leadership elections and what not
        // and allows it to process anything added to the queue in the meantime
        // return nullopt when no io needs to be done and return the head of needed_actions if there's anything there
        std::optional<io_action<Action, DomainAction>> crank_machine(std::chrono::milliseconds time_passed) noexcept{

            // if we have any log entries that are committed but not applied we should apply them
            while(lastApplied < commitIndex){
                this->log_result.apply(this->log[lastApplied+1]);
                this->lastApplied++;
            }
            // check if we're due another election, if so become a candidate, increment term and ask for votes (even if current state is leader)
            if(this->calling_election()){

            }
            switch (this->currentState){
                // if we're a follower we're done I think
                case mode::follower:
                    if(this->needed_actions.size() == 0) return std::nullopt;
                    else{
                        io_action<Action, DomainAction> ret = this->needed_actions.front();
                        this->needed_actions.pop_front();
                        return ret;
                    }
                break;
                // candidates should check if they should restart the election and if so increment term and ask for votes
                case mode::candiate:

                break;
                case mode::leader:
                    if(this->commitIndex < this->log.size()){
                        // send out a message to all the siblings about the uncommitted logs
                    }
                    else if(time_passed+this->time_since_heartbeat > HEARTBEAT_TIMER){
                        this->prepend_heartbeat();
                        this->time_since_heartbeat = 0;
                    }
                    
                break;
            }
            // unreachable
        }
    };
}
