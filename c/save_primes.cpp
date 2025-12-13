#include "PrimesGenerator.h"
#include "PrimesRepository.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>

volatile bool keep_running = true;

void handle_signal(int signal)
{
    switch(signal)
    {
        case SIGINT:
        case SIGTERM:
            keep_running = false;
            break;
        default:
            break;
    }
}

int main(int argc, char **argv)
{
    int rc;
    std::map<uint64_t, uint64_t> prime_multiples_map;

    /* Set signal handler. */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    PrimesRepository *primes_repository = PrimesRepository::create();

    if (primes_repository->connect(
                "DRIVER=SQLITE3;Database=./primes.sqlite3;") != 0)
    {
        printf(
                "Repository connect failed: %s\n",
                primes_repository->getLastError());
        return 1;
    }

    if (primes_repository->readSavedPrimeMultiples(prime_multiples_map) != 0)
    {
        printf(
                "Failed to read saved primes from repository: %s\n",
                primes_repository->getLastError());
        return 1;
    }

    PrimesGenerator primes_generator(prime_multiples_map);

    while (keep_running)
    {
        uint64_t next_prime = primes_generator.next();
        printf("%llu\n", next_prime);

        if (primes_repository->savePrimeMultiplesMap(
                    primes_generator.getPrimeMultiplesMap()) != 0)
        {
            printf(
                    "Saving prime failed: %s\n",
                    primes_repository->getLastError());
            return 1;
        }
    }

    if (primes_repository->disconnect() != 0)
    {
        printf(
                "Repository disconnect failed:\n",
                primes_repository->getLastError());
        return 1;
    }

    return 0;
}
