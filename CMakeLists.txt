# I'm also using this project to get a better understanding of CMake
# the project structure takes heavily after https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
# This file takes heavily from https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(raft_cpp_impl
    VERSION 0.1
    LANGUAGES CXX)

# setting this so compile_commands.json is generated for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(FetchContent)

# for CI and other cases
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/archive/refs/tags/boost-1.83.0.tar.gz
    URL_MD5 3018e207fd693a3096131db98977e3d6
    #GIT_REPOSITORY https://github.com/boostorg/boost.git
    #GIT_TAG boost-1.83.0
    #OVERRIDE_FIND_PACKAGE
    #GIT_SHALLOW true
)
FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/10.2.1.tar.gz
    URL_MD5 dc09168c94f90ea890257995f2c497a5
    #GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    #GIT_TAG 10.2.0
    #OVERRIDE_FIND_PACKAGE
    #GIT_SHALLOW true
)

find_package(Boost REQUIRED)
# grabbing fmt lib because atm std::format and std::print support is a bit sketchy
find_package(fmt REQUIRED)

# lib src code
add_subdirectory(src)

# app source code
add_subdirectory(apps)

