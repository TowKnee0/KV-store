#ifndef BTree_H
#define BTree_H

#include <optional>
#include <vector>
#include <string>
#include <memory>
#include "../../io/io.h"
#include "../sst.h"
#include "../../buffer/buffer.h"
#include <limits>

using namespace std;

class BTree : public Sst
{
private:
    string filename; // Which file the btree is stored in
    string dbname;
    shared_ptr<IO> io_handler;
    shared_ptr<Buffer> buffer;
    int B;
    bool use_binary_search = false;
    
    // Metadata. Metadata is stored in the last page of the file.
    // You can get the last page by calling io_handler->numPagesInFile(filename)
    int B_TREE_SIZE_INDEX = 0;
    int B_TREE_HEIGHT_INDEX = 1;
    int B_TREE_NUM_DATA_INDEX = 2;

    // Helpers
    int num_leaf_pages = 0;
    int num_data = 0;
    int INF = numeric_limits<int>::max();
    vector<pair<int, int>> internal_nodes;

    // Varaibles which need to be found
    int height = -1;

    vector<pair<int, int>> contructInternalNodes();
    int searchInPage(int key, int page_location);
    optional<pair<int, int>> getItem(int key, int page_location);
    void loadMetadata();
    vector<pair<int, int>> constructMetadata();

    int getCloestIndex(int key);
    optional<int> btreeGet(int key);

    // Methods for binary search
    optional<pair<vector<pair<int, int>>, int>> findPageWithValueEqualToOrGraterThan(int key);
    optional<pair<pair<int, int>, int>> FindEqualToOrNextLargestInPage(int key, vector<pair<int, int>> page);
    optional<int> binarySearchGet(int key);

public:
    BTree(string filename, string dbname, shared_ptr<IO> io_handler, bool use_binary_search = false, shared_ptr<Buffer> buffer = nullptr);
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