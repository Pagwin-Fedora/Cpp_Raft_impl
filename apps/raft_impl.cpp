#include <iostream>
#include <vector>
#include <string>

int main(int argc, char *argv[]){
    std::vector<std::string> args(argv+1, argv+argc);
    
    for(auto& arg:args) std::cout << arg << std::endl;

    return 0;
}
