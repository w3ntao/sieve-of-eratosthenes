cmake_minimum_required(VERSION 3.10)
project(primes)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads)
add_executable(primes main.cpp)
target_link_libraries(primes Threads::Threads)
