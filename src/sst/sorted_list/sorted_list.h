
// When a memtable reaches capacity we need to transform it into an Sst (Sorted String Table).
// The memtable will output all the key-value pairs to a sorted file which we will save in our storage device.
// We will handle only integer primitive datatype for simplicity. i.e. both the key and value is an integer

#ifndef SortedList_H
#define SortedList_H

#include <optional>
#include <vector>
#include <string>
#include <memory>
#include "../../io/io.h"
#include "../sst.h"

using namespace std;

class SortedList : public Sst
{
private:
    string filename; // Which file the Sst is stored in
    string dbname;
    shared_ptr<IO> io_handler;
    optional<pair<vector<pair<int, int>>, int>> findPageWithValueEqualToOrGraterThan(int key);
    optional<pair<pair<int, int>, int>> FindEqualToOrNextLargestInPage(int key, vector<pair<int, int>> page);

public:
    SortedList(string filename, string dbname, shared_ptr<IO> io_handler);
    int write(const vector<pair<int, int>> &data);
    optional<int> get(int key); // Search the corresponding value for this key.

    vector<int> getKeyData(int key);
    int getNumDataPages();
    string getFilename();
};

#endif