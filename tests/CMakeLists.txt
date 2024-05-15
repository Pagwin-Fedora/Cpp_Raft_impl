add_executable(raft_init raft_init.cpp)

target_compile_features(raft_init PRIVATE cxx_std_17)

target_link_libraries(raft_init PRIVATE raft_interface)

add_test(NAME initial_election COMMAND raft_init)
