#include "PrimesGenerator.h"

#include <gmock/gmock.h>
#include <stdint.h>

class PrimesGeneratorTests: public ::testing::Test
{
};

TEST_F(PrimesGeneratorTests, Generator_FindsFirstValidPrimes)
{
    std::vector<uint64_t> first_primes_list = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31
    };

    PrimesGenerator primes_generator;


    for (int expected_value: first_primes_list)
        ASSERT_EQ(primes_generator.next(), expected_value);
}

TEST_F(PrimesGeneratorTests, GivenEmptyMap_Generator_FindsFirstValidPrimes)
{
    std::map<uint64_t, uint64_t> saved_primes_map;

    std::vector<uint64_t> first_primes_list = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31
    };

    PrimesGenerator primes_generator(saved_primes_map);


    for (int expected_value: first_primes_list)
        ASSERT_EQ(primes_generator.next(), expected_value);
}

TEST_F(
        PrimesGeneratorTests, GivenMapOfKnownPrimes_Generator_FindsNextPrimes)
{
    std::map<uint64_t, uint64_t> saved_primes_map;

    std::vector<uint64_t> saved_primes_list = {
        2, 3, 5, 7, 11, 13
    };

    std::vector<uint64_t> next_primes_list = {
        17, 19, 23, 29, 31
    };

    for (uint64_t saved_prime: saved_primes_list) {
        saved_primes_map.insert(std::make_pair(saved_prime, saved_prime));
    }

    PrimesGenerator primes_generator(saved_primes_map);

    for (uint64_t expected_value: next_primes_list)
        ASSERT_EQ(primes_generator.next(), expected_value);
}
