#include <iostream>
#include <vector>
#include <string>
#include <ranges>
#include <fmt/core.h>

int main(int argc, char *argv[]){
    std::vector<std::string> args(argv+1, argv+argc);

    //simple test to make sure C++20, this should read from stdin and print out the lines it reads
    auto lines = std::ranges::subrange(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>())
        | std::views::lazy_split('\n')
        | std::views::transform([](auto view){
                std::string s;
                for(char c:view) s.push_back(c);
                return s;
                });

    for(auto line:lines) fmt::println("{}",line);

    return 0;
}
