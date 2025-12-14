cmake_minimum_required(VERSION 3.14)

project(primes)

add_library(primesgenerator src/impl/PrimesGenerator.cpp)
target_include_directories(primesgenerator PUBLIC src/include)

add_library(primesrepository src/impl/PrimesRepository.cpp)
target_include_directories(primesrepository PUBLIC src/include)

add_executable(test_primes src/test/test_primes.cpp)
target_link_libraries(test_primes primesgenerator gmock gmock_main gtest)

add_executable(save_primes src/impl/save_primes.cpp)
target_link_libraries(save_primes primesgenerator primesrepository odbc)
