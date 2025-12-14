# Primes Generators

Implements a solution in a couple languages for generating prime numbers and saving them to a database.

Both solutions generate primes using the [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) algorithm, except that the generator continually increases the "limit" and records all found prime numbers to test against the current limit.

Known primes are saved to a database, along with their last tested multiples. When the respective applications are run, the previous state of these prime numbers and multiples is loaded from the database, allowing the application to continue from wherever it was stopped.

Both solutions currently default to a SQLite database, creating a file named `primes.sqlite3`.

## Python Application

The Python solution consists of one file, `primes.py`.
- `python3 -m unittest primes.py` will run unit tests.
- `./primes.py` will generate and save prime numbers.

Requires SQLAlchemy for database operations. See its `requirements.txt` (e.g. `pip install -r requirements.txt`)

## C++ Application

The C++ solution requires the following to be installed:

- CMake version 3.14 or later
- unixODBC and an appropriate driver (the `save_primes` application expects a SQLite driver by default).
- GoogleTest and GoogleMock

Running `cmake` and `make` will build the applications that test the generator and save primes.
- `test_primes` will run unit tests.
- `./save_primes` will generate and save prime numbers.
