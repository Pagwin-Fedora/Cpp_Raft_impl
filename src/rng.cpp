#include <raft/rng.hpp>


std::seed_seq raft::make_seed_seq(std::size_t num_count){
    auto seeds = raft::make_seeds(num_count);

    return std::seed_seq(seeds.begin(), seeds.end());
}

// make a vector of numbers to give to seed_seq to seed rng with
std::vector<std::uint32_t> raft::make_seeds(std::size_t count){
    auto nums = std::vector<std::uint32_t>();
    nums.reserve(count);

    // wanted to use something like this but couldn't get seed_seq to take the iterator
    // namespace views = std::views;
    //auto iter = views::iota(0) | views::take(num_count)
    //    | views::transform([](auto _){return std::random_device()();});
    std::random_device rng;
    for(std::size_t i = 0;i<count;i++){
        nums.push_back(rng());
    }
    return nums;
}
