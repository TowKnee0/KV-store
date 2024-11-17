#include "bloom_filter.h"
#include <cmath>

using namespace std;

/**
 * @brief Calculates the optimal number of hash functions for a Bloom filter.
 * 
 * M * ln(2)
 *
 * @param numBitsPerEntry The number of bits per entry (m/n) in the Bloom filter.
 * @return The optimal number of hash functions to use.
 */
int BloomFilter::calculateOptimalNumHashes(int numBitsPerEntry) {
    return ceil(numBitsPerEntry * log(2));
}

/**
 * @brief Constructs a BloomFilter object.
 * 
 * @param bitsPerEntry The number of bits allocated per entry in the Bloom filter.
 */
BloomFilter::BloomFilter(int bitsPerEntry) {
    this->bitsPerEntry = bitsPerEntry;
    this->numHashFunctions = calculateOptimalNumHashes(bitsPerEntry);
    this->bitmap = 0;
    for (int i = 0; i < this->numHashFunctions; i++) {
        this->hashSeeds.push_back(i);
    }
}

/**
 * @brief Checks if the Bloom filter possibly contains the given key.
 *
 * @param key The key to check for existence in the Bloom filter.
 * @return true if in the filter, false if not. False positives are possible.
 */
bool BloomFilter::hasKey(int key) {
    for (int seed : this->hashSeeds) {
        uint32_t hash = xxh::xxhash<32>(to_string(key), seed);
        uint32_t bit_index = hash % this->bitsPerEntry;
        uint32_t selected_bit = 1 << bit_index;
        if ((this->bitmap & selected_bit) == 0) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Adds a key to the Bloom filter.
 *
 * @param key The key to be added to the Bloom filter.
 */
void BloomFilter::addKey(int key) {
    for (int seed : this->hashSeeds) {
        uint32_t hash = xxh::xxhash<32>(to_string(key), seed);
        uint32_t bit_index = hash % this->bitsPerEntry;
        uint32_t selected_bit = 1 << bit_index;
        this->bitmap |= selected_bit;
    }
}