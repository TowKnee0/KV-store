
// When a memtable reaches capacity we need to transform it into an SST (Sorted String Table).
// The memtable will output all the key-value pairs to a sorted file which we will save in our storage device.
// We will handle only integer primitive datatype for simplicity. i.e. both the key and value is an integer

#ifndef SST_H
#define SST_H

#include <optional>
#include <vector>
#include <string>

using namespace std;

struct entry {
    int key;
    int value;
};

class SST {
    private:
        string filename; // Which file the SST is stored in
        string dbname;
    public:
        SST(string filename, string dbname); // The SST should be assigned a unique name.
        int write(const vector<pair<int, int>>& data);
        optional<int> get(int key); // Search the corresponding value for this key.
        vector<entry>* scan(int rangeStart, int rangeEnd); // Return all currently existing key-value pairs (as entry struct) that are in the given target range
};

#endif