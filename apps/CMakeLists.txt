# based heavily on https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/apps/CMakeLists.txt
add_executable(raft_impl raft_impl.cpp)
target_compile_features(raft_impl PRIVATE cxx_std_23)
