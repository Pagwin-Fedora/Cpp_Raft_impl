# I'm also using this project to get a better understanding of CMake
# the project structure takes heavily after https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
# This file takes heavily from https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(raft_cpp_impl
    VERSION 0.1
    LANGUAGES CXX)

# setting this so compile_commands.json is generated for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


find_package(Boost REQUIRED)
# grabbing fmt lib because atm std::format and std::print support is a bit sketchy
find_package(fmt REQUIRED)

# lib src code
add_subdirectory(src)

# app source code
add_subdirectory(apps)

