# I'm also using this project to get a better understanding of CMake
# the project structure takes heavily after https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
# This file takes heavily from https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(raft_cpp_impl
    VERSION 0.1
    LANGUAGES CXX)

find_package(Boost REQUIRED)

# grabbing fmt lib because not sure if GCC or Clang support C++23 or 20 whichever adds fmt and don't care to investigate enough to be sure
include(FetchContent)
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 5.3.0)
FetchContent_MakeAvailable(fmtlib)

# lib src code
add_subdirectory(src)

# app source code
add_subdirectory(apps)

