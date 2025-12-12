#!/usr/bin/env python3
from sqlalchemy import (
        Column, Integer, MetaData, Table,
        create_engine, select, insert, update)

import signal
import sqlalchemy
import time
import unittest

metadata = MetaData()

primes_table = Table(
        "primes",
        metadata,
        Column("value", Integer, primary_key=True),
        Column("last_multiple", Integer))


def generate_primes(prime_multiples_map: dict={}) -> int:

    if len(prime_multiples_map) == 0:
        prime_multiples_map[2] = 2
        yield 2

        prime_multiples_map[3] = 3
        yield 3

        next_max = 5
    else:
        next_max = max(prime_multiples_map.keys())

    while True:

        is_next_max_prime = True

        for prime, multiple in prime_multiples_map.items():
            
            while multiple < next_max:
                multiple += prime

            prime_multiples_map[prime] = multiple
                
            if multiple == next_max:
                is_next_max_prime = False
                break

        if is_next_max_prime:
            prime_multiples_map[next_max] = next_max
            yield next_max

        next_max += 2



class TestGeneratePrimes(unittest.TestCase):

    def test_generate_primes_returns_first_primes(self):

        first_primes = [
            2,
            3,
            5,
            7,
            11,
            13,
            17,
            19,
            23
        ]

        primes_generator = generate_primes()

        for prime in first_primes:

            with self.subTest(prime=prime):
                self.assertEqual(next(primes_generator), prime)

def main():

    state = {'keep_running': True}

    def stop_on_signal(signal, frame):
        state['keep_running'] = False
    signal.signal(signal.SIGINT, stop_on_signal)

    db_engine = create_engine('sqlite:///primes.sqlite3')

    metadata.create_all(db_engine)

    with db_engine.connect() as db_connection:

        prime_multiples_map = {
            prime: multiple for prime, multiple in db_connection.execute(
                select(
                    primes_table.c.value,
                    primes_table.c.last_multiple))
        }
    
    primes_generator = generate_primes(
            prime_multiples_map=prime_multiples_map)

    while state['keep_running']:

        # Save a copy of the current primes and multiples.
        # The copy can be shallow.
        map_copy = prime_multiples_map.copy()

        next_save_time = time.time() + 1

        while state['keep_running']:

            next_prime = next(primes_generator)
            print(next_prime)
            if time.time() >= next_save_time:
                break

        with db_engine.begin() as db_connection:

            # The generator updates the primes map.
            # persist any changes.

            for prime, multiple in prime_multiples_map.items():

                if prime not in map_copy:
                    db_connection.execute(
                            insert(primes_table).values(
                                value=prime, last_multiple=multiple))

                elif map_copy[prime] != multiple:
                    db_connection.execute(
                        update(primes_table)
                        .where(primes_table.c.value == prime)
                        .values(last_multiple=multiple))

if __name__ == '__main__':
    main()
