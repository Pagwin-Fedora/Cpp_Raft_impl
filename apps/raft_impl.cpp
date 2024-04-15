#include <iostream>
#include <vector>
#include <string>
#include <ranges>
#include <fmt/core.h>

int main(int argc, char *argv[]){
    std::vector<std::string> args(argv+1, argv+argc);
    
    for(auto& arg:args) fmt::println("{}",arg);

    return 0;
}
