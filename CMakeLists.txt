# I'm also using this project to get a better understanding of CMake
# the project structure takes heavily after https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
# This file takes heavily from https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(raft_cpp_impl
    VERSION 0.1
    LANGUAGES CXX)

# setting this so compile_commands.json is generated for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#include(FetchContent)

# for CI and other cases
#FetchContent_Declare(
#    fmt
#    URL https://github.com/fmtlib/fmt/releases/download/10.2.1/fmt-10.2.1.zip
#    URL_MD5 04e266ad52659480d593486a17eed804
#    DOWNLOAD_EXTRACT_TIMESTAMP true
#    #GIT_REPOSITORY https://github.com/fmtlib/fmt.git
#    #GIT_TAG 10.2.0
#    OVERRIDE_FIND_PACKAGE
#    #GIT_SHALLOW true
#    USES_TERMINAL_DOWNLOAD TRUE 
#    GIT_PROGRESS TRUE   
#    DOWNLOAD_NO_EXTRACT FALSE
#)
#FetchContent_MakeAvailable(fmt)

# not willing to make this work for now
find_package(Boost REQUIRED)
# grabbing fmt lib because atm std::format and std::print support is a bit sketchy
find_package(fmt REQUIRED)

# lib src code
add_subdirectory(src)

# app source code
add_subdirectory(apps)

