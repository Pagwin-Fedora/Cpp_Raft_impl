#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <optional>
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

    template <typename return_type> using rpc_ret = std::pair<term_t, return_type>;

    struct leader_state{
        std::vector<std::uint64_t> nextIndex;
        std::vector<std::uint64_t> matchIndex;
    };

    class base_action {
        index_t idx;
        term_t term;
    };
    enum mode{
        follower,
        candiate,
        leader
    };
    class leader_status{
        mode current_state;

    };

    enum io_action_variants{
        send_log,
        request_vote,
        domain_action
    };

    // Action is subclass of base_action which we can use for various things
    // DomainAction is a value that's used on state machine inside
    // io_action is trying to be a tagged union telling the IO impl what to do
    template <typename Action, typename DomainAction>
    class io_action{
        static_assert(std::is_base_of_v<base_action, Action>(),"ActionType must inherit from base_action to ensure it has associated state");
        io_action_variants variant;
        std::vector<Action> send_log_val;
        DomainAction domain_action_val;
        id_t target;
        public:
        io_action_variants get_variant(){return this->variant;}
        std::vector<Action> const& send_log(){return this->send_log_val;}
        id_t const& get_target(){return this->target;}
        DomainAction const& domain_action(){return this->domain_action_val;}
    };

    template<typename Action, typename InnerMachine>
    class state_machine{
        static_assert(std::is_base_of_v<base_action, Action>(),"ActionType must inherit from base_action to ensure it has associated state");

        // normally I don't put _t after type names but id, index and term can easily be var names so clarification seemed useful

        // names correspond to names in paper
        id_t myId;
        std::uint64_t currentTerm;
        std::optional<id_t> votedFor;
        std::vector<Action> log;
        index_t commitIndex;
        index_t lastApplied;
        leader_status leadership_view;

        // implementation details for IO and state machine
        std::unordered_set<id_t> servers;
        InnerMachine log_result;

        public:
        // I really don't like that templated functions need to go in headers but oh well
        // might be sensible to trim the arg count down via a struct or class which contains all this and builder pattern
        rpc_ret<bool> append_entries(term_t term, id_t leaderId, index_t prevLogIndex, term_t prevLogTerm, std::vector<Action> const& entries, index_t leaderCommit) noexcept {
            if(term < this->currentTerm) return std::make_pair(std::move(this->currentTerm), false);
            if(log.size() < prevLogIndex || std::get<0>(log[prevLogIndex]) != prevLogTerm) return std::make_pair(std::move(term), false);

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

            return std::make_pair(std::move(term), true);
        }

        rpc_ret<bool> request_votes(term_t term, id_t candidateId, index_t lastLogIndex, term_t lastLogTerm) noexcept{
            if(term < this->currentTerm) return std::make_pair(this->currentTerm, false);

            if(!votedFor.has_value() || votedFor.value() == candidateId){

                bool got_vote = lastLogTerm >= std::max_element(this->log.begin(), this->log.end(), [](Action a, Action b){return a.term < b.term;});

                if(got_vote) this->votedFor.emplace(candidateId);
                return std::make_pair(std::move(term), got_vote);
            }
            else {
                return std::make_pair(std::move(term), false);
            }
        }
        
        // 
        template<typename InputIt>
        std::variant<std::optional<std::optional<id_t>>, std::size_t> enqueue_actions(InputIt start, InputIt end){
            // TODO: make it so actions are put into the log and whatever state is incremented
            if(*start.idx != this->log.size()) return this->log.size();
            if(!this->volatile_leader_state.has_value()) return std::make_optional(this->votedFor);
            else return std::nullopt;
        }

        // method that tells the machine how long it's been since the last crank for leadership elections and what not
        // and allows it to process anything added to the queue in the meantime
        std::vector<io_action<base_action, char>> crank_machine(std::chrono::milliseconds time_passed){
            return std::vector<io_action<base_action, char>>();
        }
    };
}
