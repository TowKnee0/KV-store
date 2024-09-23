
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

struct entry
{
    int key;
    int value;
};

class Sst
{
private:
    string filename; // Which file the Sst is stored in
    string dbname;

public:
    Sst(string filename, string dbname); // The Sst should be assigned a unique name.
    int write(const vector<pair<int, int>> &data);
    optional<int> get(int key); // Search the corresponding value for this key.

    /*
     * Scans the SST file and returns all key-value pairs that are in the given target range.
     * The range is exclusive of both endpints.
     *
     * Parameters:
     * - rangeStart: the start of the range (exclusive)
     * - rangeEnd: the end of the range (exclusive)
     *
     * Returns:
     * - A vector of key-value pairs that are in the given range.
     */
    vector<entry> *scan(int rangeStart, int rangeEnd);
};

#endif