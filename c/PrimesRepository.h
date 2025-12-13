#ifndef PRIMES_REPOSITORY_H
#define PRIMES_REPOSITORY_H

#include <stdint.h>
#include <map>

/**
 * Connects to a database, and creates a table (if it does not exist) of prime
 * numbers. Specifically two columns, representing a state of prime number
 * generation.
 *
 * This is used in combinationwith ith the ::PrimesGenerator to build a table
 * of known prime numbers.
 *
 * The table columns are:
 *
 * - 'value' - A prime number value
 * - 'last_multiple' - The largest multiple of the prime number value, which is
 *   used by the ::PrimesGenerator to help identify more numbers which are
 *   (not) primes.
 *
 *   This class leverages ODBC to perform database operations -- the ::connect
 *   method requires an ODBC connection string.
 *
 *   Call the ::create method to instantiate a repository.  To destroy a
 *   repository, delete it.
 */
class PrimesRepository
{
    public:

        /**
         * @brief Connect to a database.
         *
         * @param odbc_connect_string ODBC connection string specifying the
         * database to connect to.
         *
         * @return 0 on success, -1 on error (call ::getLastError to get error
         * text)
         */
        virtual int connect(const char *odbc_connect_string) = 0;

        /**
         * @brief Disconnect from a databse.
         *
         * @return 0 on success, -1 on error (call ::getLastError to get error
         * text)
         */
        virtual int disconnect() = 0;

        /** @brief Save a given mapping of prime values to their lagest
         * multiples used to find new prime numbers. The ::PrimesGenerator
         * provides such a map.
         *
         * @param prime_multiples_map The mapping of primes to multiples
         *
         * @return 0 on success, -1 on error (call ::getLastError to get error
         * text) */
        virtual int savePrimeMultiplesMap(
                const std::map<uint64_t, uint64_t> &prime_multiples_map) = 0;


        /**
         * @brief Read a mapping of prime values to their largest multiples
         * from the database. This can be passed to the ::PrimesGenerator to
         * initialize it to a known state.
         *
         * @param prime_multiples_map Map to populate
         *
         * @return 0 on success, -1 on error (call ::getLastError to get error
         * text) */
        virtual int readSavedPrimeMultiples(
                std::map<uint64_t, uint64_t> &prime_multiples_map) = 0;

        /**
         * @brief Get text of the last error to occur when calling one of this
         * class' other methods.
         *
         * The error text is valid until the next call to a method that may
         * return error text (indicated by method documentation).
         *
         * @return String representing the last error to occur
         */
        virtual const char *getLastError() = 0;

        static PrimesRepository *create();

    protected:

        /** Mapping of known primes to the largest multiple that has been used
         * by e.g. ::PrimesGenerator to find new prime numbers. */
        std::map<uint64_t, uint64_t> _last_prime_multiples_map;
};

#endif
