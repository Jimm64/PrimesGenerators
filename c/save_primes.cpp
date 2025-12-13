/**
 * @file save_primes.cpp
 *
 * @brief Finds prime numbers and saves them to a database. */

#include "PrimesGenerator.h"
#include "PrimesRepository.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    const char *odbc_connection_string;

    if (argc == 1)
        odbc_connection_string = "DRIVER=SQLITE3;Database=./primes.sqlite3;";
    else if (argc == 2)
        odbc_connection_string = argv[1];
    else
    {
        printf("Usage: $s [ODBC connection string]\n");
        return 1;
    }

    /* Set signal handler. */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    PrimesRepository *primes_repository = PrimesRepository::create();

    if (primes_repository->connect(odbc_connection_string) != 0)
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
                "Repository disconnect failed: %s\n",
                primes_repository->getLastError());
        return 1;
    }

    delete primes_repository;
    return 0;
}
