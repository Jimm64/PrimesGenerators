#ifndef PRIMES_GENERATOR_H
#define PRIMES_GENERATOR_H

#include <stdint.h>
#include <map>

class PrimesGenerator
{
    public:
        PrimesGenerator();

        PrimesGenerator(
                const std::map<uint64_t, uint64_t> &known_prime_multiples_map);

        /** Get next prime value. */
        uint64_t next();

        const std::map<uint64_t, uint64_t> getPrimeMultiplesMap();


    protected:

        /** Saves known prime values and a multiple thereof. */
        std::map<uint64_t, uint64_t> _prime_multiples_map;

        /** The next possible prime value. */
        uint64_t _next_possible_prime;
};

#endif
