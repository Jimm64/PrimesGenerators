#include "PrimesGenerator.h"
#include "gmock/gmock.h"

#include <stdint.h>

class PrimesGeneratorTests: public ::testing::Test
{
};

TEST_F(PrimesGeneratorTests, GeneratorFindsFirstValidPrimes)
{
    std::vector<uint64_t> first_primes_list = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31
    };

    PrimesGenerator primes_generator;


    for (int expected_value: first_primes_list)
        ASSERT_EQ(primes_generator.next(), expected_value);
}
