#ifndef BTree_H
#define BTree_H

#include <optional>
#include <vector>
#include <string>
#include <memory>
#include "../../io/io.h"
#include "../sst.h"
#include <limits>

using namespace std;

class BTree : public Sst
{
private:
    string filename; // Which file the btree is stored in
    string dbname;
    shared_ptr<IO> io_handler;
    int B;

    // Helpers
    int num_leaf_pages = 0;
    int num_data = 0;
    int INF = numeric_limits<int>::max();


    // Varaibles which need to be found
    int height = -1;

    void contructInternalNodes();

public:
    BTree(string filename, string dbname, shared_ptr<IO> io_handler);
    int write(const vector<pair<int, int>> &data);
    optional<int> get(int key); // Search the corresponding value for this key.

    /*
     * Scans the SST file and returns all key-value pairs that are in the given target range.
     * The range is EXCLUSIVE of both endpints.
     *
     * Parameters:
     * - rangeStart: the start of the range (exclusive)
     * - rangeEnd: the end of the range (exclusive)
     *
     * Returns:
     * - A vector of key-value pairs that are in the given range.
     */
    vector<pair<int, int>> scan(int rangeStart, int rangeEnd);
};

#endif