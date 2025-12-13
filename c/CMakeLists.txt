cmake_minimum_required(VERSION 3.14)

project(primes)

add_library(
        primesgenerator
        PrimesGenerator.cpp)
add_library(
        primesrepository
        PrimesRepository.cpp)

add_executable(
        test_primes test_primes.cpp)
target_link_libraries(
        test_primes primesgenerator gmock gmock_main gtest)

add_executable(
        save_primes save_primes.cpp)
target_link_libraries(
        save_primes primesgenerator primesrepository odbc)
