#ifndef PRIMES_REPOSITORY_H
#define PRIMES_REPOSITORY_H

#include <stdint.h>
#include <map>

class PrimesRepository
{
    public:

        virtual int connect(const char *odbc_connect_string) = 0;

        virtual int disconnect() = 0;

        virtual int savePrimeMultiplesMap(
                const std::map<uint64_t, uint64_t> &prime_multiples_map) = 0;

        virtual int readSavedPrimeMultiples(
                std::map<uint64_t, uint64_t> &prime_multiples_map) = 0;

        virtual const char *getLastError() = 0;

        static PrimesRepository *create();

    protected:

        std::map<uint64_t, uint64_t> _last_prime_multiples_map;
};

#endif
