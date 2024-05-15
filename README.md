# Raft Algorithm implementation

At time of writing this is an incomplete implementation of the [Raft](https://raft.github.io/raft.pdf) consensus algorithm. It does build and run however it fails to successfully elect a leader.

## Dependencies:

- Boost
- CMake >= 3.10

## How to build/run

1. `cmake --B build`
2. `cmake --build build`
3. the executable can be found in `./build/apps/raft_impl` and is run with `$exe_location $server_id` where server id is one of the numbers in the `sockets` file which needs to be in your current working directory when you run the executable

Sidenote: running an instance of the program will use 100% of a CPU core which is intentional

However if you have tmux installed 5 instances of the program can be automatically run in different tmux panels via the `start_raft.sh` shell script.

## Some code explanation

There are 4 directories with code in them, `include`, `src`, `apps` and `tests`.

### `include`

Second paragraph says where the raft algorithm implementation is and explains it.

`include` holds the header file `lib.hpp` which has a majority of the code if you exclude the tests. The reason for this is because templates were used for a lot of the implementation to allow it to be more generic. In that header everything goes into a namespace `raft` so in case this code gets used in the future it won't need to get a namespace added to avoid name conflicts. `id_t`, `index_t` and `term_t` are defined so the code is more readible and have the `_t` suffix due to those being reasonable names for variables. `base_action` is a class that's setup to be a base class for an action that can be applied to a state machine that raft manages. `base_state_machine` is that state machine. `mode` ise used in the raft implementation as the type of value which specifies what mode the raft machine is in of follower, candidate or leader. `io_action_variants`, `io_action`, `rpc_ack`, `vote_request_state` and `send_log_state` will be explained along with how the IO works which will be explained in the section on the `apps` folder.

`state_machine` is the implementation of the raft algorithm and is a template which takes template arguments corresponding to the type of the action which gets applied to the underlying state machine, the underlying state machine and a type that was intended to represent actions that could do IO that aren't directly releated to Raft but isn't used anywhere so can be ignored. The members don't  follow a consistent naming scheme due to some coming from the raft paper and some being added as part of our implementation.

- `myId` is the id of the raft server and is used to tell other servers who is sending a message, it could've been held by the io layer instead but it was decided to have it in the raft implementation.

- `currentTerm` is self explanatory.

- `following` is the variable used to specify which raft server the current server is either voting for or is currently believed to be the leader.

- `siblings` is a list of all the other raft servers.

- `replicatedIndices` is used to track how much of the log has been replicated on each sibling.

- `log` is the variable holding onto the full list of all actions that could or are applied to the underlying state machine.

- `commitIndex` is the currently committed log entry, with a log entry only being committed when a majority of servers have received it and which will always be in the log.

- `lastApplied` is the index of the last log entry to be applied to the underlying state machine and will never be greater than the `commitIndex` in a working implementation.

- `currentState` specifies which of leader, follower or candidate the server currently is.

- `log_result` is the underlying state machine which log entries get applied to.

- `electionTimeout` is the length of time the server will wait before calling an election.

- `time_since_heartbeat` measures how long the server has gone without a heartbeat.

- `votes_received_counter` is a count of how many votes the server has gotten from other servers

- finally `needed_actions` is used to hold onto a queue of actions the raft implementation wants the io layer to do with front entries being removed at the end of `crank_machine`

There's also various procedures that `state_machine` has

- `ack`, `send_log` and `request_vote` are helpers for adding a particular io action onto the queue

- `append_entries` corresponds to the AppendEntries RPC from the raft paper and is called by the IO layer whenever that RPC is received, it appends entries to the log if the log is up to date enough and the term of the leader sending those logs is high enough as well as reseting the heartbeat timer

- `request_votes` corresponds to the RequestVote RPC from the raft paper and is called by the IO layer whenever that RPC is received, it performs the checks on term and the log entry indice/term provided by the candidate to decide if it gets it's vote and can continue asking for more

- `ack_rpc` is an rpc added in this implementation and called by the IO layer as needed to be the way the raft implementation can find out about a response to the prior RPCs

- `swap_follower` is a helper method which changes the raft implementation to a follower and purges `io_actions` in the queue that are now redundant due to it being a follower

- `enqueue_actions` might be dead code but if not is meant to be called on a leader from the io layer to add log entries to be distributed

- `prepend_heartbeat` is a helper method which is used to make sure a heartbeat is sent out regularly and isn't delayed by other actions in the queue

- `calling_election` is a helper method used in `crank_machine` to determin of a server should call an election at that time

- `crank_machine` is the method called by the io layer to tell the machine about the advancement of time, to allow it to perform work that doesn't occur in an RPC and to allow it to tell the io layer what action it needs done

After that there are helper methods which are mostly implemented in `src` barring templates which I'll talk about in that section anyways.

### `src`

`src` is where C++ files which don't have templates go, it only has `lib.cpp` which implements the following methods

- `parse_x`, parse a string into whatever `x` is, this was done instead of overloading streams because streams are annoying

- `display_x` turn an `x` into a string which can be parsed by parse

### `apps`

`apps` is where the executable and by extension the io layer lives.

Before the io layer is run some setup is done so there's a mapping between ids and server unix sockets and the machine the process corresponds to is known and the implemenation is initialized. After that comes the IO layer.

The io layer is most of the code in `raft_impl.cpp` and what it does at a high level is simple. It does all the io and passes that information to the raft implementation so the raft implementation doesn't do IO allowing it to be tested easily and change communication methodology if so desired. It is probably unecessary overengineering for this project but it was fun. To do all of this the io layer spawns 3 threads in addition to the main thread via `std::async`. The first thread listens on a unix socket for raft RPC messages so it can pass them along to the implementation as needed. The second thread reads from stdin so a demo can be done and the third thread exists so the leader can be found for demo purposes. The main thread meanwhile starts polling the implementation in a loop via the `crank_machine` method keeping track of the the time between each loop and passing that in as an argument while taking the action it's given back and doing it by connecting to the relevant unix socket.

### `tests`

The tests directory has a couple of files `raft_follower_init.cpp` and `raft_leader_init.cpp` which were written to try and work through the kinks of the implemenation without needing to deal with IO at the same time. `raft_follower_init` tests to see if the follower behaves correctly up until a leader is selected and `raft_leader_init` checks to see if all the nodes become the leader correctly given suffiecient circumstances. Each file is compiled into an executable and run, if it exits with a 0 it succeeded if it exits with a 1 it failed with an error message. Both tests pass right now but the actual demo doesn't work for unknown reasons.

If you wanna run the tests yourself you can either find the executables in the `./build/tests` directory after building or you can use ctest (assuming it's installed) via `ctest --test-dir build` after building everything

## Misc

There is a conan file with the hope that it'd make it easy enough to deal with dependencies that the code wouldn't be limited to just the standard library and maybe boost as dependencies but that didn't pan out. Haven't bothered removing it though.
