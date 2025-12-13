#ifndef PRIMES_REPOSITORY_H
#define PRIMES_REPOSITORY_H

#include <stdint.h>
#include <map>

class PrimesRepository
{
    public:

        virtual int connect(const char *odbc_connect_string) = 0;

        virtual int disconnect() = 0;

        virtual int savePrime(uint64_t value) = 0;

        virtual int readSavedPrimes(
                std::map<uint64_t, uint64_t> &primes_map) = 0;

        virtual const char *getLastError() = 0;

        static PrimesRepository *create();
};

#endif
