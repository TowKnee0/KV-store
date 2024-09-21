
// When a memtable reaches capacity we need to transform it into an SST (Sorted String Table).
// The memtable will output all the key-value pairs to a sorted file which we will save in our storage device.
// We will handle only integer primitive datatype for simplicity. i.e. both the key and value is an integer

#ifndef SST_H
#define SST_H

#include "../memtable/bst.h" // Include the BST header
#include <optional>
#include <vector>
#include <string>

using namespace std;

enum ErrorCode {SUCCESS, WRITE_FAILURE};

struct entry {
    int key;
    int value;
};

class SST {
    private:
        string filename; // Which file the SST is stored in
        ErrorCode writeToFile(); // For writing the SST contents to storage
    public:
        SST(const vector<pair<int, int>>& data, const string& filename); // The SST should be assigned a unique name.
        optional<int> get(int key); // Search the corresponding value for this key.
        vector<entry>* scan(int rangeStart, int rangeEnd); // Return all currently existing key-value pairs (as entry struct) that are in the given target range
};

#endif