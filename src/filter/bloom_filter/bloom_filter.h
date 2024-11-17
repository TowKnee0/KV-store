#include "../filter.h"
#include <cstdint>
#include "../../../include/xxhash.hpp"

class BloomFilter : public Filter {
    private:
        int bitsPerEntry;
        int numHashFunctions;
        
        int calculateOptimalNumHashes(int numBitsPerEntry);

        // Max 32 bits per entry.
        uint32_t bitmap;

        // Instead of computing k hash functions,
        // use the same hash function, but use k different seeds.
        vector<int> hashSeeds;

    public:
        BloomFilter(int bitsPerEntry);
        virtual bool hasKey(int key);
        virtual void addKey(int key);
};