
// When a memtable reaches capacity we need to transform it into an Sst (Sorted String Table).
// The memtable will output all the key-value pairs to a sorted file which we will save in our storage device.
// We will handle only integer primitive datatype for simplicity. i.e. both the key and value is an integer

#ifndef Sst_H
#define Sst_H

#include <optional>
#include <vector>
#include <string>
#include <memory>

using namespace std;

class Sst
{
public:
    virtual int write(const vector<pair<int, int>> &data) = 0;
    virtual optional<int> get(int key) = 0; // Search the corresponding value for this key.

    /**
     * @brief Scans the SST file and returns all key-value pairs that are in the given target range.
     * The range is exclusive of both endpoints.
     *
     * @param rangeStart The start of the range (exclusive).
     * @param rangeEnd The end of the range (exclusive).
     * @return vector<pair<int, int>> A vector of key-value pairs that are in the given range.
     */
    virtual vector<pair<int, int>> scan(int rangeStart, int rangeEnd) = 0;

    /**
     * @brief Get the index, page, key and value of the key in the BTree.
     * Returns the next largest key if the key is not found.
     * If there is no next largest key, returns -1 for all values.
     *
     * @param key The key to find in the BTree.
     * @return {int, int, int, int} The index, page, key and value of the key in the BTree.
     */

    virtual vector<int> getKeyData(int key) = 0;

    virtual ~Sst() = default;
};

#endif