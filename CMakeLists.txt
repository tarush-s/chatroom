cmake_minimum_required(VERSION 3.20)

project(chatroom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

enable_testing()

add_executable(smart_fd_test
    utility/smart_fd_test.cpp
)

target_link_libraries(smart_fd_test
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)

add_test(
    NAME smart_fd_test
    COMMAND smart_fd_test
)