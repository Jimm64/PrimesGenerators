#ifndef PRIMES_GENERATOR_H
#define PRIMES_GENERATOR_H

#include <stdint.h>
#include <map>

/** Generates prime numbers in increasing sequence.  */
class PrimesGenerator
{
    public:

        /**
         * @brief Construct a generator with no known primes.
         */
        PrimesGenerator();

        /**
         * @brief Start generator from a map of known state of existing primes
         * and their largest tested multiples.
         *
         * @param known_prime_multiples_map Map of known primes to their
         * largest tested multiples.
         */
        PrimesGenerator(
                const std::map<uint64_t, uint64_t> &known_prime_multiples_map);

        /**
         * @brief Return the next prime value.
         *
         * @return The next found prime value.
         */
        uint64_t next();

        /**
         * @brief Return the current state of existing primes and their largest
         * tested multiples.
         *
         * @return Map of existing primes and multiples.
         */
        const std::map<uint64_t, uint64_t> getPrimeMultiplesMap();


    protected:

        /** Saves known prime values and a multiple thereof. */
        std::map<uint64_t, uint64_t> _prime_multiples_map;

        /** The next possible prime value. */
        uint64_t _next_possible_prime;
};

#endif
