#ifndef PRIMES_REPOSITORY_H
#define PRIMES_REPOSITORY_H

#include <stdint.h>

class PrimesRepository
{
    public:

        virtual int connect() = 0;

        virtual int disconnect() = 0;

        virtual int savePrime(uint64_t value) = 0;

        virtual const char *getLastError() = 0;

        static PrimesRepository *create();
};

#endif
