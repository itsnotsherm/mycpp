
cmake_minimum_required(VERSION 3.22)

project(mycpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(mycpp INTERFACE)
target_include_directories(mycpp INTERFACE include)

find_package(Threads REQUIRED)

add_executable(examples src/main.cpp)
target_link_libraries(examples PRIVATE mycpp Threads::Threads)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY  https://github.com/google/googletest.git
        GIT_TAG         v1.17.0
        GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(googletest)
enable_testing()
