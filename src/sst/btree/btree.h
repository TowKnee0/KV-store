#ifndef BTree_H
#define BTree_H

#include <optional>
#include <vector>
#include <string>
#include <memory>
#include "../../io/io.h"
#include "../sst.h"

using namespace std;

class BTree : public Sst
{
private:
    string filename; // Which file the btree is stored in
    string dbname;
    shared_ptr<IO> io_handler;
    int B;

    string INTENRAL_NODE_SUFFIX = "I";
    string LEAF_NODE_SUFFIX = "L";

    // Helpers
    int num_data_pages = 0;
    int num_data = 0;

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