#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

// on page 4 there's a 1 page reference sheet https://raft.github.io/raft.pdf
// TODO: implement everything under Rules for servers
namespace raft{
    // stubbed, not sure how to define command type yet
    class action{};

    class leader_state{
        std::vector<std::uint64_t> nextIndex;
        std::vector<std::uint64_t> matchIndex;
    };

    template<typename CommandType>
    class state_machine{

        // normally I don't put _t after type names but id, index and term can easily be var names so clarification seemed useful
        using id_t = std::uint64_t;
        using index_t = std::uint64_t;
        using term_t = std::uint64_t;

        template <typename return_type> using rpc_ret = std::pair<term_t, return_type>;

        // names correspond to names in paper
        id_t myId;
        std::uint64_t currentTerm;
        std::optional<id_t> votedFor;
        std::vector<action> log;
        index_t commitIndex;
        index_t lastApplied;
        std::optional<leader_state> volatile_leader_state;


        public:
        // might be sensible to trim the arg count down via a struct or class which contains all this and builder pattern
        rpc_ret<bool> append_entries(term_t term, id_t leaderId, index_t prevLogIndex, std::vector<action> const& entries, index_t leaderCommit) noexcept;

        rpc_ret<bool> request_votes(term_t term, id_t candidateId, index_t lastLogIndex, term_t lastLogTerm);
        

    };
}
