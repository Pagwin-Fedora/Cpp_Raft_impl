# based heavily on https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/apps/CMakeLists.txt

add_executable(raft_impl raft_impl.cpp)

target_compile_features(raft_impl PUBLIC cxx_std_20)

target_link_libraries(raft_impl PUBLIC raft_interface)

target_link_libraries(raft_impl PUBLIC Boost::boost)

target_link_libraries(raft_impl PUBLIC fmt::fmt)
