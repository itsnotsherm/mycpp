
cmake_minimum_required(VERSION 3.22)

project(mycpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)
include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY  https://github.com/google/googletest.git
        GIT_TAG         v1.17.0
        GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(test_vector tests/test_vector.cpp)
target_link_libraries(test_vector GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_vector)
