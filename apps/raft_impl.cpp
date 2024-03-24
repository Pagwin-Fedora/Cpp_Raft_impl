#include <vector>
#include <string>
#include <fmt/core.h>

int main(int argc, char *argv[]){
    std::vector<std::string> args(argv+1, argv+argc);
    fmt::println("Hello {}", args[0]);
    return 0;
}
