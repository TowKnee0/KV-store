
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

enum ErrorCode {SUCCESS};

struct entry {
    int key;
    int value;
};

class SST {
    private:
        time_t creation_time; // When was this SST created
        string filename; // Which file the SST is stored in
        vector<entry> entries; // What is contained in this SST
     
    public:
        SST(const vector<entry>& data, const string& filename); // The SST should be assigned a unique name.
        optional<int> get(int key); // Search the corresponding value for this key

        // It is not known in advance how many key-value pairs are in the target range
        // So we will need to return a pointer to an array of results along with the size of this array
        pair<vector<entry>*, int> scan(int rangeStart, int rangeEnd);
};

#endif