
cmake_minimum_required(VERSION 3.22)

project(mycpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY  https://github.com/google/googletest.git
        GIT_TAG         v1.17.0
        GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(googletest)
enable_testing()
