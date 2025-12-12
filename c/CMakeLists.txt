cmake_minimum_required(VERSION 3.14)

project(primes)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0)
FetchContent_MakeAvailable(googletest)

add_library(
        primesgenerator
        PrimesGenerator.cpp)

add_executable(
        test_primes test_primes.cpp)
target_link_libraries(
        test_primes primesgenerator gmock gmock_main)
