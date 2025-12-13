#include "PrimesGenerator.h"

PrimesGenerator::PrimesGenerator()
{
    _next_possible_prime = 2;
}

PrimesGenerator::PrimesGenerator(
        const std::map<uint64_t, uint64_t> &known_prime_multiples_map)
{
    if (known_prime_multiples_map.size() > 0)
    {
        _next_possible_prime = known_prime_multiples_map.rbegin()->first + 2;
        _prime_multiples_map = known_prime_multiples_map;
    }
    else
        _next_possible_prime = 2;
}

uint64_t PrimesGenerator::next()
{
    /*
     * Implement an Eratosthenes sieve with an ever-increasing limit -
     *
     * Maintain a map of each known prime to a value that is a multiple of that
     * prime.
     *
     * Whenver we search for the next prime, start with a possible prime value,
     * then iterate over that map, multiplying each value in that map as long
     * as it is less than that possible prime value.
     *
     * If one of those multiples is ever found to be equal to the possible
     * prime value, then the value is not prime after all.
     *
     * If we iterate over all values in the map, and all multiples have become
     * greater than the possible prime value, then we know the value is indeed
     * prime, so return it.
     *
     * */

    if (_next_possible_prime == 2)
    {
        _prime_multiples_map.insert(std::make_pair(2,2));
        _next_possible_prime = 3;
        return 2;
    }
    else if (_next_possible_prime == 3)
    {
        _prime_multiples_map.insert(std::make_pair(3,3));
        _next_possible_prime += 2;
        return 3;
    }

    while (true)
    {
        bool is_next_value_prime = true;

        for(
                std::map<uint64_t, uint64_t>::iterator map_iter
                =  _prime_multiples_map.begin();
                map_iter != _prime_multiples_map.end();
                ++map_iter)
        {
            while (map_iter->second < _next_possible_prime)
                map_iter->second += map_iter->first;

            if (map_iter->second == _next_possible_prime)
            {
                is_next_value_prime = false;
                break;
            }
        }

        if (is_next_value_prime)
        {
            uint64_t return_value = _next_possible_prime;
            _prime_multiples_map.insert(std::make_pair(
                        _next_possible_prime, _next_possible_prime));
            _next_possible_prime += 2;
            return return_value;
        }
        else
            _next_possible_prime += 2;
    }
}
